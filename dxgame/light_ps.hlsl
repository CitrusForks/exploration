// Filename: light.ps

#include "cpp_hlsl_defs.h"

// a random number function of some sort that I found on the net, ported from apparently glsl:

// Input: It uses texture coords as the random number seed.
// Output: Random number: [0,1), that is between 0.0 and 0.999999... inclusive.
// Author: Michael Pohoreski
// Copyright: Copyleft 2012 :-)

float random( float2 p )
{
  // We need irrationals for pseudo randomness.
  // Most (all?) known transcendental numbers will (generally) work.
  const float2 r = float2(
    23.1406926327792690,  // e^pi (Gelfond's constant)
     2.6651441426902251); // 2^sqrt(2) (Gelfond–Schneider constant)
  return frac( cos( fmod( 123456789., 1e-7 + 256. * dot(p,r) ) ) );  
}

// and this nonsense is by me:
float2 rand2( float2 p )
{
    return float2(random(p), random(float2(p.y + countbits(p.x), p.x + countbits(p.y))));
}


// globals:

//Texture2D diffuseTexture[15] : register(t0); // uggggh. nah.

Texture2D diffuseTexture : register(t0); // aka the texture map
Texture2D normalMap : register(t1); // yep, normals
Texture2D specularMap : register(t2); // per-pixel specular values

// shadow maps:
Texture2D shadowMap[NUM_SPOTLIGHTS+2] : register(t3); 
// this would be way nicer as a Texture2DArray but we can't use a Texture2DArray as a DepthStencilBuffer target... 
// copying a bunch of shadowmaps from individual texture would be a lot of extra work for the GPU just to make this shader code a little prettier
// so for now, this is what's happening.

// anisotropic filtering sampler:
SamplerState SampleType : register(s0);
// unfiltered sampler:
SamplerState SampleUnfiltered : register(s1);

// this buffer needs to be split up into separate lights and material buffers:
cbuffer MaterialBuffer : register(b0)
{
	float4 ambientColor;
	float4 diffuseColor;
        float4 specularColor;
        float specularPower;
	bool useNormalMap;
	bool useSpecularMap;
	float padding;
}

cbuffer LightBuffer : register(b1)
{
        float3 lightDirection; // directional light; one is enough for now?
	float time;            // this isn't strictly light but it's useful for goofy effects
	float4 cameraPos;
	float4 spotlightPos[NUM_SPOTLIGHTS];  // spotlights!
	float4 spotlightDir[NUM_SPOTLIGHTS];  // float3 would take up 4 floats anyway, might as well make it explicit
	float4 spotlightEtc[NUM_SPOTLIGHTS];  // arrays are packed into 4-float elements anyway so this is {Cos(angle), constant attenuation, linear, quadratic}
        float4 ambientLight; // color
        float4 diffuseLight; // color, can also encode intensity obviously
	uint numLights;        
};
// end of globals

struct PixelInputType
{
        float4 position : SV_POSITION;
	float4 modelPos : MODELPOS;
	float4 worldPos : WORLDPOS;
        float2 tex : TEXCOORD0;
	nointerpolation uint texNum : TEXINDEX;                    // which texture to use
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 viewDirection : VIEWDIR;
	float4 shadowUV[NUM_SPOTLIGHTS+2] : SHADOWUV;
};


static const float PI = 3.14159265358979;

float sampleShadowMap(float2 uv, uint which)
{
// because we can't use a texture array with a depth buffer, we live with this:
#define SAMPLESHADOWMAP(i) if (i == which) return shadowMap[i].Sample(SampleUnfiltered, uv).r;
SAMPLESHADOWMAP(0)
SAMPLESHADOWMAP(1)
SAMPLESHADOWMAP(2)
SAMPLESHADOWMAP(3)
SAMPLESHADOWMAP(4)
SAMPLESHADOWMAP(5)
#undef SAMPLESHADOWMAP
// :|
return 0.0; 
}


float isSpotlightShadow(float4 lightClipSpaceCoordinates, uint whichShadow, float2 jitter)
{
    float shadowSample = sampleShadowMap(
            jitter + float2(
             0.5 * lightClipSpaceCoordinates.x / lightClipSpaceCoordinates.w + 0.5, 
            -0.5 * lightClipSpaceCoordinates.y / lightClipSpaceCoordinates.w + 0.5),  // clip -> NDC -> viewport transforms
        whichShadow
    );

    if (shadowSample < lightClipSpaceCoordinates.z / lightClipSpaceCoordinates.w) return true; // clip -> NDC via divide by w and this matches the data in the data in the map buffer
    else return false;
}


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 color;
        float4 specular = 0.0f;
	uint texNum = input.texNum;
	float3 tangent = input.tangent;
        float4 actualSpecularColor = specularColor;;
	//tangent = normalize(input.tangent);

	//return 1.0-pow(input.position.zzzz,2); // ghostly distance-visualization world

	//return float4(0, dot(input.tangent, input.normal), 0, 1)*10; // uncomment for error visualization
	//return float4(tangent.x, tangent.y, tangent.z, 1); // uncomment for amazing technicolor vomit shader

	textureColor = diffuseTexture.Sample(SampleType, input.tex);

	clip(textureColor.a - 0.95); // discard transparent pixels

	float specularMultiplier = 1.0f;

	// check the specular map
	if (useSpecularMap)
	{
	    specularMultiplier = specularMap.Sample(SampleType, input.tex).r;

            if (specularColor.x == 0.0f && specularColor.y == 0.0f && specularColor.z == 0.0f) actualSpecularColor = float4(0.1, 0.1, 0.1, 0.1);
	} 


	float3 normal;

	// make sure tangent is orthonormal (to normal)
	// essentially subtracts any component of tangent along the direction of the normal [Luna, figure 18.6]
	if (useNormalMap)
	{
		//normal = normalize(input.normal);
		//tangent = normalize(input.tangent);
		tangent = tangent - input.normal * dot(tangent, input.normal);
		float3 binormal = normalize(cross(input.normal, tangent));
		float3x3 tangentToWorld = float3x3(tangent, binormal, input.normal);

		float3 mapped_normal = normalMap.Sample(SampleType, input.tex).xyz*2 - float3(1,1,1); // rescale the values to [-1,+1] range
		//return mapped_normal;
	
		normal = normalize(mul(mapped_normal, tangentToWorld));
	} else
	{
		normal = normalize(input.normal);
	}

	// vector from the eye
	float3 viewDirection = normalize(cameraPos.xyz - input.worldPos.xyz );

	// Set the default output color to the ambient light value for all pixels.
        color = ambientColor * ambientLight;

	// Initialize the specular color.
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Invert the light direction for calculations.
        lightDir = -lightDirection;

        // Calculate the amount of light on this pixel.
        lightIntensity = saturate(dot(normal, lightDir));

	if(lightIntensity > 0.0f)
        {
            float ds;

            int totalShadow = 0;
            uint mapNum = NUM_SPOTLIGHTS+1;

            // get shadowmap coordinates in local high def shadowmap:
            float2 uv = float2(0.5 * input.shadowUV[mapNum].x / input.shadowUV[mapNum].w +
                0.5, -0.5 * input.shadowUV[mapNum].y + 0.5);

            if (uv.x > 0.01 && uv.x < 0.99 && uv.y > 0.01 && uv.y < 0.99) // are the coordinates in the map?
            {
                ds = 1.0/2048; // high LOD map dimension
                //color.r = 1.0;
            } else
            {
                mapNum = NUM_SPOTLIGHTS;
                //uv = float2(input.shadowUV[mapNum].x * 0.5 + 0.5, -0.5 * input.shadowUV[mapNum].y + 0.5);
                ds = 1.0/4096; // low LOD map dimension
                //color.g = 1.0;
            }
            
            //return sampleShadowMap(input.shadowUV[NUM_SPOTLIGHTS+1], NUM_SPOTLIGHTS+1);

            if (isSpotlightShadow(input.shadowUV[mapNum], mapNum, float2(0,0)))
            {
                totalShadow += 1;
                if (isSpotlightShadow(input.shadowUV[mapNum], mapNum, float2(ds,ds))) totalShadow += 1;
                if (isSpotlightShadow(input.shadowUV[mapNum], mapNum, float2(-ds,-ds))) totalShadow += 1;
                if (isSpotlightShadow(input.shadowUV[mapNum], mapNum, float2(-ds,ds))) totalShadow += 1;
                if (isSpotlightShadow(input.shadowUV[mapNum], mapNum, float2(ds,-ds))) totalShadow += 1;
                if (isSpotlightShadow(input.shadowUV[mapNum], mapNum, float2(0,ds))) totalShadow += 1;
                if (isSpotlightShadow(input.shadowUV[mapNum], mapNum, float2(0,-ds))) totalShadow += 1;
                if (isSpotlightShadow(input.shadowUV[mapNum], mapNum, float2(-ds,0))) totalShadow += 1;
                if (isSpotlightShadow(input.shadowUV[mapNum], mapNum, float2(ds,0))) totalShadow += 1;

            }

            if (totalShadow < 9)
            {
                float shadowMultiplier = 1.0f - (1.0f/9.0f) * totalShadow;
                // Determine the final diffuse color based on the diffuse color and the amount of light intensity.
                color += (diffuseColor * lightIntensity * diffuseLight * shadowMultiplier);

	        // Saturate the ambient and diffuse color.
		color = saturate(color);

		//a graphical representation of the slight difference between the results of interpolating view direction and interpolating world position for recalculating view direction
		//float3 r = abs(normalize(viewDirection) - input.viewDirection); // actually, this seems broken right now
		//float3 r = abs(normal);
		//float4 rr = {r.x, r.y, r.z, 1.0f};
		//return rr;

		if (specularMultiplier > 0.00001f)
		{
			// calculate half-vector for Blinn-Phong specular model
			float3 H = normalize(-lightDirection + viewDirection);
			// calculate specular reflection based on dot product of half-vector and normal vector, along with material data
			specular = shadowMultiplier * specularMultiplier * actualSpecularColor * pow(saturate(dot(H, normal)), specularPower);
		}
            }
        }

	// spotlight calculations
	for (uint i = 0; i < 2; ++i) // once per light...
	{
		if (spotlightPos[i].w == 0.0) break; // out of lights in the scene
		
		float3 sourceToPixel = input.worldPos.xyz - spotlightPos[i].xyz;
		float distToLight = distance(input.worldPos, spotlightPos[i]);
		sourceToPixel = sourceToPixel / distToLight; // not sure whether the HLSL compiler will optimize distance(), normalize() like so; TODO: check asm output? :/

		float beamAlignment = dot(spotlightDir[i], sourceToPixel);
		float beamCosAngle = spotlightEtc[i].x;

		if (beamAlignment > 0.0 && beamAlignment > beamCosAngle)
		{
			// well, a spotlight could be hitting this pixel
			// check for shadow, though
                        uint shadowTotal = 0;
                        if (isSpotlightShadow(input.shadowUV[i], i, float2(0,0))) continue; //shadowTotal++;
                        //if (isSpotlightShadow(input.shadowUV[i], i, float2((0.0011), 0))) shadowTotal++;
                        //if (isSpotlightShadow(input.shadowUV[i], i, float2((0.0011),0))) shadowTotal++;
                        //if (isSpotlightShadow(input.shadowUV[i], i, float2(0, (0.0011)))) shadowTotal++;
                        //if (isSpotlightShadow(input.shadowUV[i], i, float2(0, (0.0011)))) shadowTotal++;

                        //if (shadowTotal == 5) continue;

			float3 toLight = -sourceToPixel;
			float spotlightIntensity = dot(normal, toLight);

			if (spotlightIntensity > 0.0)
			{
				float attenuation = 1.0 / (spotlightEtc[i].y + spotlightEtc[i].z * distToLight + spotlightEtc[i].w * pow(distToLight,2));

                                attenuation *= 1-(shadowTotal/5.0); // modify attenuation by shadow factor because it's convenient

				//beamAlignment = pow(beamAlignment,100); // XXX this seems like stupid way of doing this but the books do it pretty much like this, it turns out

				//float fallOff = saturate((beamAlignment*1.00001-beamCosAngle)/(1-beamCosAngle)); // maybe this way is stupid too; 1.0001 was chosen experimentally
				float fallOff = sin(PI/1.3 * (beamAlignment-beamCosAngle)/(1-beamCosAngle)); // maybe this way is stupid too; 1.0001 was chosen experimentally
				
				spotlightIntensity *= fallOff * diffuseColor * attenuation; 

				color = color + spotlightIntensity;

				if (specularMultiplier > 0.0000001f)
				{
					// calculate half-vector for Blinn-Phong specular model
					float3 H = normalize(toLight + viewDirection);
					// calculate specular reflection based on dot product of half-vector and normal vector, along with material data
					specular += specularMultiplier * attenuation * fallOff * actualSpecularColor * pow(saturate(dot(H, normalize(normal))), specularPower);
				}
			}
		}
	}

	//return saturate(color + specular); // pure light value visualization
	//return saturate(float4(0, color.r, specular.g, 1)); // separate ambient vs. specular visual

        // Multiply the texture pixel and the input color to get the textured result.
        color = color * textureColor;

	// Add the specular component last to the output color.
        color = saturate(color + specular);

       return color;
}
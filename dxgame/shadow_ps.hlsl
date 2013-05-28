// Filename: light.ps

// globals:
Texture2D diffuseTexture[15] : register(t0);
Texture2D normalMap : register(t1);
SamplerState SampleType;

// this buffer needs to be split up into separate lights and material buffers:
cbuffer MaterialBuffer : register(b0)
{
	float4 ambientColor;
	float4 diffuseColor;
    float4 specularColor;
    float specularPower;
	bool useNormalMap;
	float2 padding;
}

#define NUM_SPOTLIGHTS 4

cbuffer LightBuffer : register(b1)
{
    float3 lightDirection; // directional light; one is enough for now?
	float time;            // this isn't strictly light but it's useful for goofy effects
	float4 cameraPos;
	float4 spotlightPos[NUM_SPOTLIGHTS];  // spotlights!
	float4 spotlightDir[NUM_SPOTLIGHTS];  // float3 would take up 4 floats anyway, might as well make it explicit
	float4 spotlightEtc[NUM_SPOTLIGHTS];  // arrays are packed into 4-float elements anyway so this is {Cos(angle), constant attenuation, linear, quadratic}
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
};


static const float PI = 3.14159265358979;

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{

	float textureAlpha = diffuseTexture[0].Sample(SampleType, input.tex).a;

	clip(textureAlpha - 0.95); // discard transparent pixels
	
	// the transparency discard incurs a texture load per pixel per shadowmap... perhaps it's too costly?

	// do not actually render anything, we only want the depth buffer to use as a shadow map
	return float4(0,0,0,0);
}
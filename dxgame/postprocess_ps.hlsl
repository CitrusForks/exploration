Texture2D shaderTexture;
SamplerState SampleType : register(s1); // register one contains the non-filtering sampler

//
// Globals -- shared with main shader for C++ code reuse but we probably only use time here
//
#include "cpp_hlsl_defs.h"

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
    float2 tex : TEXCOORD0;
};


float pow2(float x)
{
	return x*x;
}

float distanceSq(float4 A, float4 B)
{
	return pow2(B.x - A.x) + pow2(B.y - A.y) + pow2(B.z - A.z) + pow2(B.w - A.w);
}


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PostProcPixelShader(PixelInputType input) : SV_TARGET
{
	float2 baseOffset;
	const float2 Half = {0.5, 0.5};

	float2 ghostImageOffset;
	float2 ghost2;

	baseOffset = input.tex - Half;
	float2 tex = input.tex - baseOffset * 0.05; // zoom in a little to have some extra source image "off-screen"

#if 1
	// wavy ghost image effect
	float4 textureColor = shaderTexture.Sample(SampleType, tex);
	float darkness = 1.0 - saturate(distance(textureColor, float4(0,0,0,0))/1.5);

	ghostImageOffset = baseOffset * 0.01;
	ghostImageOffset.x += 0.01 * cos(ghostImageOffset.x * 1000 + time*7) * darkness;
	ghostImageOffset.y += 0.01 * cos(ghostImageOffset.y * 1000 + time*11) * darkness;

	//ghost2 = baseOffset * 0.03 * sin(time*3.5);
	
	textureColor += shaderTexture.Sample(SampleType, saturate(tex + ghostImageOffset));
	//textureColor += shaderTexture.Sample(SampleType, saturate(tex + ghost2));
	textureColor /= 2;
#endif

#if 0
    // crude edge detection!
	float4 textureColor = shaderTexture.Sample(SampleType, tex);
	tex.x += 0.001;
	tex.y += 0.001;
	textureColor -= shaderTexture.Sample(SampleType, tex);
#endif

#if 0
	// chromatic aberration
	float4 textureColor;
	textureColor.x = shaderTexture.Sample(SampleType, tex - baseOffset * 0.01).x; // red is smudged outward
	textureColor.y = shaderTexture.Sample(SampleType, tex - baseOffset * 0.005).y; // green less so
	textureColor.z = shaderTexture.Sample(SampleType, tex).z;
#endif

#if 0
	// strange distortion?
	float4 textureColor;

	tex += baseOffset * pow(distance(input.tex, Half), 3) * 0.5;

	if (tex.x > 0.9 || tex.x < 0.01 || tex.y > 0.9 || tex.y < 0.01) return float4(0,0,0,0);

	textureColor = shaderTexture.Sample(SampleType, tex);
#endif

#if 0
    // ugly blur :|
	float4 textureColor = shaderTexture.Sample(SampleType, tex);

	float4 textureColorX = shaderTexture.Sample(SampleType, tex + float2(1.0/1024,0)) + shaderTexture.Sample(SampleType, tex - float2(1.0/1024,0));

	float4 textureColorY = shaderTexture.Sample(SampleType, tex + float2(0,1.0/768)) + shaderTexture.Sample(SampleType, tex - float2(1.0/768,0));

	if (distanceSq(textureColor, textureColorX) > distanceSq(textureColor, textureColorY))
	{
		textureColor += textureColorX + textureColorY/2;
	} else
	{
		textureColor += textureColorY + textureColorX/2;
	}

	textureColor /= 4;
#endif

#if 0
	// effects off
	float4 textureColor = shaderTexture.Sample(SampleType, tex); 
#endif

	return textureColor;
}
// Filename: light.ps

#include "cpp_hlsl_defs.h"

// a random number function of some sort that I found on the net, which I ported from glsl:

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

TextureCube skyBox : register(t0); // it's a box full of sky perhaps? who knows. WHO KNOWS! UGHHHHH.

// shadow maps:
Texture2D shadowMap[NUM_SPOTLIGHTS+2] : register(t3); 
// this would be way nicer as a Texture2DArray but we can't use a Texture2DArray as a DepthStencilBuffer target... 
// copying a bunch of shadowmaps from individual texture would be a lot of extra work for the GPU just to make this shader code a little prettier
// so for now, this is what's happening.

// anisotropic filtering sampler:
SamplerState SampleAnisotropic : register(s0);
// linear-filtered + borders sampler:
SamplerState SampleShadows : register(s1);
// unfiltered
SamplerState SampleUnfiltered : register(s2);
// unfiltered
SamplerState SampleLinear : register(s3);

// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb509644%28v=vs.85%29.aspx (modified)
SamplerComparisonState FilterShadows : register(s4);
#if 0
// this doesn't seem to work, thank you documentation example:
{
   // sampler state
   Filter = MIN_MAG_LINEAR_MIP_POINT;
   AddressU = BORDER;
   AddressV = BORDER;
   BorderColor = 0.0;

   // sampler comparison state
   ComparisonFunc = LESS;
   ComparisonFilter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
};
#endif

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
        float4 spotlightCol[NUM_SPOTLIGHTS];  // spotlight color!
        float4 ambientLight; // color
        float4 diffuseLight; // color, can also encode intensity obviously
	uint numLights;        
};
// end of globals

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 uvw : TEXCOORD;
};


static const float PI = 3.14159265358979;


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
    return pow(skyBox.Sample(SampleAnisotropic, input.uvw), 2);
    //return float4(0, 1, 0, 1);
}

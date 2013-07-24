// Filename: light.vs
// Originally from rastertek.com tutorials, heavily modified

#include "cpp_hlsl_defs.h"

//
// Globals
//
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix lightVP[NUM_SPOTLIGHTS+2]; // world coordinates -> light shadow map coordinates
	uint numLights;
};

cbuffer CameraBuffer
{
    float4 cameraPosition;
	float time;
	uint effect;
};


//
// Typedefs
//
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    uint texNum : TEXINDEX;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 uvw : TEXCOORD;
};


//
// Vertex Shader
//
PixelInputType main(VertexInputType input)
{
    PixelInputType output;

    matrix viewMod = viewMatrix;
    viewMod._41 = viewMod._42 = viewMod._43 = 0; // remove translation vector from matrix

    output.position = mul(input.position, viewMod); // the skycube requires no world matrix... it is already where it was meant to be
    output.position = mul(output.position, projectionMatrix).xyww; // set z to w so that z buffer value will be set to max distance (1!)

    output.uvw = float3(input.position.x, input.position.y, input.position.z);


    return output;
}
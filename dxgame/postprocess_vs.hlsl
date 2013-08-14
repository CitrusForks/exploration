/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CameraBuffer
{
    float4 cameraPosition;
	//float padding;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


PixelInputType PostProcVShader(VertexInputType input)
{
    PixelInputType output;

	// Calculate the position of the vertex against the ortho matrix
    output.position = mul(input.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
	//output.tex = input.position.xy;
	output.tex.y = 1 + output.tex.y; // our texture loader is flipping the V coordinate which is valid... but negative values are a hassle in the pixel shader

    return output;
}
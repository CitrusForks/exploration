////////////////////////////////////////////////////////////////////////////////
// Filename: light.vs
////////////////////////////////////////////////////////////////////////////////


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
	float4 modelPos : MODELPOS;
	float4 worldPos : WORLDPOS;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewDirection : VIEWDIR;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType PostProcVShader(VertexInputType input)
{
    PixelInputType output;
	float4 worldPosition;

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the ortho matrix
    output.position = mul(input.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
	//output.tex = input.position.xy;
	output.tex.y = 1 + output.tex.y; // our texture loader is flipping the V coordinate which is valid... but negative values are a hassle in the pixel shader
    
	output.normal = input.normal;

    return output;
}
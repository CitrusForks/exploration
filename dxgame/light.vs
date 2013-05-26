// Filename: light.vs
// Originally from rastertek.com tutorials, heavily modified

//
// Globals
//
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
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
	float4 modelPos : MODELPOS;
	float4 worldPos : WORLDPOS;
    float2 tex : TEXCOORD0;
	nointerpolation uint texNum : TEXINDEX;                    // which texture to use
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 viewDirection : VIEWDIR;
};


//
// Misc
//
matrix rotateAboutY(float angle)
{
	float4 col1 = {cos(angle), 0, -sin(angle), 0};
	float4 col2 = {0, 1, 0, 0};
	float4 col3 = {sin(angle), 0, cos(angle), 0};
	float4 col4 = {0, 0, 0, 1};
	
	return matrix(col1, col2, col3, col4); 
}



//
// Vertex Shader
//
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
	float4 worldPosition;

	output.modelPos = input.position;
	output.texNum = input.texNum;

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	float4 localPosition = input.position;
	float3 normal = input.normal;
	float3 tangent = input.tangent;

	if (effect == 1)
	{ // twist the object
		matrix convolution = rotateAboutY(3.14159 * sin(time + localPosition.y/2));
		localPosition = mul(localPosition, convolution);
		normal = mul(normal, (float3x3)convolution);
		tangent = mul(tangent, (float3x3)convolution);
	}

	// Calculate the position of the vertex in world, view, and screen coordinates
	// by using the appropriate matrices
    output.position = mul(localPosition, worldMatrix);
	output.worldPos = worldPosition = output.position;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
    
	// Rotate normal vector into world coordinates
    output.normal = mul(normal, (float3x3)worldMatrix);
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);

	// Repeat for tangent vector
	output.tangent = normalize(mul(tangent, (float3x3)worldMatrix));

    // Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
    output.viewDirection = normalize(cameraPosition.xyz - worldPosition.xyz);
	
    return output;
}
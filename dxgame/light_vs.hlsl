// Filename: light.vs
// Originally from rastertek.com tutorials, heavily modified

#include "cpp_hlsl_defs.h"

#include "Structures.hlsli"


//
// Globals
//
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix bindToBoneSpace;
    matrix lightVP[NUM_SPOTLIGHTS+2]; // world coordinates -> light shadow map coordinates
    uint numLights;
    float animationTick;
};

cbuffer CameraBuffer
{
    float4 cameraPosition;
    float camTime;
    uint effect;
};

cbuffer Bones : register(b11)
{
    matrix boneTransform[MAX_BONES];
    matrix extraUnusedThing;
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


matrix quatToMatrix(float4 q) // uggghghgh see http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/
{
    // this is actually pretty slow
    float4 col1 = {q.w, -q.z, q.y, -q.x};
    float4 col2 = {q.z, q.w, -q.x, -q.y};
    float4 col3 = {-q.y, q.x, q.w, -q.z};
    float4 col4 = {q.x, q.y, q.z, q.w};
    matrix A = matrix(col1, col2, col3, col4);

    float4 col5 = {q.w, -q.z, q.y, q.x};
    float4 col6 = {q.z, q.w, -q.x, q.y};
    float4 col7 = {-q.y, q.x, q.w, q.z};
    float4 col8 = {-q.x, -q.y, -q.z, q.w};
    matrix B = matrix(col5, col6, col7, col8);

    return mul(A, B);
}



void sampleOneWeightedBone(uint index, out matrix transform)
{
    // TODO: interpolation here ffs finally?
    float3 boneLookup;

    transform = boneTransform[index];
}


void sampleBones(VertexInputType input, out float4 position, out float3 normal, out float3 tangent)
{
    uint i, n;

    float weight;
    uint index;
    matrix M;
    float4 offset;

    weight = input.boneWeights.x;
    index = input.boneIndex.x;
    sampleOneWeightedBone(index, M);
    position = weight * mul(input.position, M);
    normal = weight * mul(input.normal, (float3x3)M);
    tangent = weight * mul(input.tangent, (float3x3)M);

    weight = input.boneWeights.y;
    if (weight != 0.0f)
    {
        index = input.boneIndex.y;
        sampleOneWeightedBone(index, M);
        position += weight * mul(input.position, M);
        normal += weight * mul(input.normal, (float3x3)M);
        tangent += weight * mul(input.tangent, (float3x3)M);
    }

    weight = input.boneWeights.z;
    if (weight != 0.0f)
    {
        index = input.boneIndex.z;
        sampleOneWeightedBone(index, M);
        position += weight * mul(input.position, M);
        normal += weight * mul(input.normal, (float3x3)M);
        tangent += weight * mul(input.tangent, (float3x3)M);
    }

    weight = input.boneWeights.w;
    if (weight != 0.0f)
    {
        index = input.boneIndex.w;
        sampleOneWeightedBone(index, M);
        position += weight * mul(input.position, M);
        normal += weight * mul(input.normal, (float3x3)M);
        tangent += weight * mul(input.tangent, (float3x3)M);
    }
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

    // Change the position vector to homogeneous coordinates for proper matrix calculations.
    input.position.w = 1.0f;

    float4 localPosition = input.position;
    float3 normal = input.normal;
    float3 tangent = input.tangent;

#if 1
    if (animationTick >= 0.0f && input.boneIndex.x != -1)
    {
        matrix M;
        sampleBones(input, localPosition, normal, tangent);
    }
#endif

    if (effect == 1)
    { // twist the object
	    matrix twist = rotateAboutY(3.14159 * sin(camTime + localPosition.y/2));
	    localPosition = mul(localPosition, twist);
	    normal = mul(normal, (float3x3)twist);
	    tangent = mul(tangent, (float3x3)twist);
    }


    // Calculate the position of the vertex in world, view, and screen coordinates
    // by using the appropriate matrices
    output.worldPos = worldPosition = output.position = mul(localPosition, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    //output.position = mul(worldPosition, lightVP[NUM_SPOTLIGHTS]);
    
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
	
    for (uint i = 0; i < numLights; ++i)
    {
	    output.shadowUV[i] = mul(worldPosition, lightVP[i]);
    }

    output.shadowUV[NUM_SPOTLIGHTS] = mul(worldPosition, lightVP[NUM_SPOTLIGHTS]);    // directional light
    output.shadowUV[NUM_SPOTLIGHTS+1] = mul(worldPosition, lightVP[NUM_SPOTLIGHTS+1]);// it has two shadow maps for 2x LOD

    return output;
}

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
    float4 boneWeights : BONEWEIGHTS;
    uint4 boneIndex : BONEINDICES; 
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
    float4 shadowUV[NUM_SPOTLIGHTS+2] : SHADOWUV;
};

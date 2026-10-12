#ifndef MATERIAL_PARAMETERS_HLSLI
#define MATERIAL_PARAMETERS_HLSLI

struct MaterialVertexParameters
{
    float4 WorldPosition;
    float4 VertexColor;
    float3 Normal;
};

struct MaterialPixelParameters
{
    float4 PixelColor;
    float4 WorldPosition;
    float3 PixelNormal;
    float2 UV0;
    float Occlusion;
    float Roughness;
    float Metalness;
};

#endif
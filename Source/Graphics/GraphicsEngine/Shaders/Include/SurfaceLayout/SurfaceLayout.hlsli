#ifndef SURFACE_LAYOUT_HLSLI
#define SURFACE_LAYOUT_HLSLI

struct SurfaceData
{
    float4 Albedo;
    float3 WorldNormal;
    float2 NormalMapXY;
    float Occlusion;
    float Roughness;
    float Metalness;
    float3 Emission;
};

float3 DecodeNormal(float4 aNormalSample, float3x3 aTBN)
{
    const float2 unpackedNormal = aNormalSample.xy * 2.0 - 1.0;
    
    const float3 tangentNormal = normalize(float3(unpackedNormal, sqrt(saturate(1.0 - dot(unpackedNormal, unpackedNormal)))));
    
    return normalize(mul(tangentNormal, aTBN));
}

#endif
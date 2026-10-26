#ifndef SURFACE_PBR_HLSLI
#define SURFACE_PBR_HLSLI

#include "SurfaceLayout.hlsli"
#include "../Core/Common.hlsli"
#include "../Core/Samplers.hlsli"

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MaterialTexture : register(t2);
Texture2D EmissionTexture : register(t3);

SurfaceData EvaluateSurface(VStoPS aPixel, float2 aUV)
{
    const float4 albedo = AlbedoTexture.Sample(TrilinearWrap, aUV);
    const float4 normal = NormalTexture.Sample(TrilinearWrap, aUV);
    const float4 material = MaterialTexture.Sample(TrilinearWrap, aUV);
    const float4 emission = EmissionTexture.Sample(TrilinearWrap, aUV);
    
    const float3x3 TBN = float3x3(
        normalize(aPixel.Tangent),
        normalize(aPixel.BiNormal),
        normalize(aPixel.Normal)
    );
    
    const float emissionStrength = 50.0f;

    SurfaceData data;
    data.Albedo = albedo;
    data.WorldNormal = DecodeNormal(normal, TBN);
    data.NormalMapXY = normal.xy;
    data.Occlusion = material.r;
    data.Roughness = material.g;
    data.Metalness = material.b;
    data.Emission = emission.r * albedo.rgb * emissionStrength;
    return data;
}

#endif
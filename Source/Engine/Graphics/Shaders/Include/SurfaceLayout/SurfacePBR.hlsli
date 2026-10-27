#ifndef SURFACE_PBR_HLSLI
#define SURFACE_PBR_HLSLI

#include "SurfaceLayout.hlsli"
#include "../Core/Common.hlsli"
#include "../Core/Samplers.hlsli"

Texture2D BaseColor : register(t0);
Texture2D Normal : register(t1);
Texture2D ORM : register(t2);
Texture2D Emission : register(t3);

SurfaceData EvaluateSurface(VStoPS aPixel, float2 aUV)
{
    const float4 albedo = BaseColor.Sample(TrilinearWrap, aUV);
    const float4 normal = Normal.Sample(TrilinearWrap, aUV);
    const float4 material = ORM.Sample(TrilinearWrap, aUV);
#ifdef MATERIAL_HAS_EMISSION
    const float4 emission = Emission.Sample(TrilinearWrap, aUV);
#endif
    
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
#ifdef MATERIAL_HAS_EMISSION
    data.Emission = emission.r * albedo.rgb * emissionStrength;
#else
    data.Emission = float3(0, 0, 0);
#endif
    return data;
}

#endif
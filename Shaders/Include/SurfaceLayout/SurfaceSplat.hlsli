#ifndef SURFACE_SPLAT_HLSLI
#define SURFACE_SPLAT_HLSLI

#include "SurfaceLayout.hlsli"
#include "../Core/Common.hlsli"
#include "../Core/Samplers.hlsli"

Texture2D BaseColor_01 : register(t0);
Texture2D Normal_01 : register(t1);
Texture2D ORM_01 : register(t2);
Texture2D BaseColor_02 : register(t3);
Texture2D Normal_02 : register(t4);
Texture2D ORM_02 : register(t5);
Texture2D BaseColor_03 : register(t6);
Texture2D Normal_03 : register(t7);
Texture2D ORM_03 : register(t8);
Texture2D BaseColor_04 : register(t9);
Texture2D Normal_04 : register(t10);
Texture2D ORM_04 : register(t11);
Texture2D SplatMap : register(t12);

#ifdef MATERIAL_HAS_HEIGHTMAP
Texture2D Height_01 : register(t13);
Texture2D Height_02 : register(t14);
Texture2D Height_03 : register(t15);
Texture2D Height_04 : register(t16);
#endif

SurfaceData EvaluateSurface(VStoPS aPixel, float2 aUV)
{
    const float3x3 TBN = float3x3(
        normalize(aPixel.Tangent),
        normalize(aPixel.BiNormal),
        normalize(aPixel.Normal)
    );
    
	// TODO: Project splatmap onto landscape or something similar
    float2 splatUV = (aPixel.WorldPosition.xz + 3150.0f) / 6300.0f;

    float4 Mask = SplatMap.Sample(TrilinearClamp, splatUV);
    Mask /= max(dot(Mask, 1.0f), 1e-5f);

#ifdef MATERIAL_HAS_HEIGHTMAP
    const float4 heights = float4(
        Height_01.Sample(TrilinearWrap, aUV).r,
        Height_02.Sample(TrilinearWrap, aUV).r,
        Height_03.Sample(TrilinearWrap, aUV).r,
        Height_04.Sample(TrilinearWrap, aUV).r);

    const float blendContrast = 0.2f;
	float4 weights = heights * Mask;
    float maxWeight = max(max(weights.r, weights.g), max(weights.b, weights.a));
    weights = max(weights - (maxWeight - blendContrast), 0.0f);
    Mask = weights / max(dot(weights, 1.0f), 1e-5f);
#endif

    float4 A_Albedo = BaseColor_01.Sample(TrilinearWrap, aUV);
    float4 A_Normal = Normal_01.Sample(TrilinearWrap, aUV);
    float4 A_Material = ORM_01.Sample(TrilinearWrap, aUV);

    float4 B_Albedo = BaseColor_02.Sample(TrilinearWrap, aUV);
    float4 B_Normal = Normal_02.Sample(TrilinearWrap, aUV);
    float4 B_Material = ORM_02.Sample(TrilinearWrap, aUV);

    float4 C_Albedo = BaseColor_03.Sample(TrilinearWrap, aUV);
    float4 C_Normal = Normal_03.Sample(TrilinearWrap, aUV);
    float4 C_Material = ORM_03.Sample(TrilinearWrap, aUV);

    float4 D_Albedo = BaseColor_04.Sample(TrilinearWrap, aUV);
    float4 D_Normal = Normal_04.Sample(TrilinearWrap, aUV);
    float4 D_Material = ORM_04.Sample(TrilinearWrap, aUV);
    
    float4 FinalAlbedo = A_Albedo * Mask.r + B_Albedo * Mask.g + C_Albedo * Mask.b + D_Albedo * Mask.a;
    float4 FinalMaterial = A_Material * Mask.r + B_Material * Mask.g + C_Material * Mask.b + D_Material * Mask.a;

    float3 A_N = DecodeNormal(A_Normal, TBN);
    float3 B_N = DecodeNormal(B_Normal, TBN);
    float3 C_N = DecodeNormal(C_Normal, TBN);
    float3 D_N = DecodeNormal(D_Normal, TBN);

    float3 FinalNormal = A_N * Mask.r + B_N * Mask.g + C_N * Mask.b + D_N * Mask.a;

    FinalNormal = normalize(FinalNormal);

    SurfaceData data;
    data.Albedo = FinalAlbedo;
    data.WorldNormal = FinalNormal;
    data.NormalMapXY = 0;
    data.Occlusion = FinalMaterial.r;
    data.Roughness = FinalMaterial.g;
    data.Metalness = FinalMaterial.b;
    data.Emission = 0;
    return data;
}

#endif
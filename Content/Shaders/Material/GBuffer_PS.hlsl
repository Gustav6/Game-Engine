#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"
#include "Include/Material.hlsli"

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MaterialTexture : register(t2);
Texture2D EmissionTexture : register(t3);

GBufferOutput main(VStoPS aPixel)
{    
    const float4 albedoMap = AlbedoTexture.Sample(TrilinearWrap, aPixel.UV0);
    const float4 normalMap = NormalTexture.Sample(TrilinearWrap, aPixel.UV0);
    const float4 materialMap = MaterialTexture.Sample(TrilinearWrap, aPixel.UV0);
    const float4 emissionMap = EmissionTexture.Sample(TrilinearWrap, aPixel.UV0);
    
    float2 unpackedNormal = normalMap.xy * 2.0 - 1.0;
    
    float3 normal = normalize(float3(unpackedNormal, sqrt(saturate(1.0 - dot(unpackedNormal, unpackedNormal)))));

    const float3x3 TBN = float3x3(
        normalize(aPixel.Tangent),
        normalize(aPixel.BiNormal),
        normalize(aPixel.Normal)
    );
    
    normal = normalize(mul(normal, TBN));
    
    float3 emission = emissionMap.r * albedoMap.rgb * 50;
    
    MaterialPixelParameters parameters;
    parameters.PixelColor = albedoMap;
    parameters.WorldPosition = aPixel.WorldPosition;
    parameters.Normal = normal;
    parameters.UV0 = aPixel.UV0;
    parameters.Occlusion = materialMap.r;
    parameters.Roughness = materialMap.g;
    parameters.Metalness = materialMap.b;
    
    Material_Pixel(parameters);
    
    GBufferOutput result;
    
    switch (FB_TypeOfRenderPass)
    {
        case 0: // sRGB pass do nothing!
            result.Albedo = float4(parameters.PixelColor.rgb, 1);
            break;
        case 1: // Rougness pass
            result.Albedo = float4(materialMap.g, materialMap.g, materialMap.g, 1.0f);
            break;
        case 2: // Metalness pass
            result.Albedo = float4(materialMap.b, materialMap.b, materialMap.b, 1.0f);
            break;
        case 3: // Ambient Occlusion texture pass
            result.Albedo = float4(materialMap.r, materialMap.r, materialMap.r, 1.0f);
            break;
        case 4: // Ambient Occlusion screen space pass
            result.Albedo = float4(1.0f, 1.0f, 1.0f, 1.0f); // TODO : FIX
            break;
        case 5: // Normals tangent space pass
            result.Albedo = float4(normalMap.xy, 0.0f, 1.0f);
            break;
        case 6: // Normals world space pass
            result.Albedo = float4(normal, 1.0f);
            break;
        case 7: // Shadows pass
            result.Albedo = float4(0.5f, 0.5f, 0.5f, 1.0f);
            parameters.Roughness = 1.0f;
            break;
        default:
            result.Albedo = float4(parameters.PixelColor.rgb, 1); // same as case 0, safe fallback
            break;
    }
    
    result.WolrdNormalDepth = float4(parameters.Normal, aPixel.ViewDepth);
    result.Material = float4(parameters.Occlusion, parameters.Roughness, parameters.Metalness, 0);
    result.Emission = float4(emission, 0);
    result.WorldPosition = parameters.WorldPosition;
    
    return result;
}
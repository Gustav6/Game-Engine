#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"
#include "Include/Material.hlsli"

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MaterialTexture : register(t2);

GBufferOutput main(VStoPS aPixel)
{
    //switch (FB_TypeOfRenderPass)
    //{
    //    case 0: // sRGB pass do nothing!
    //        break;
    //    case 1: // Rougness pass
    //        return float4(roughness, roughness, roughness, 1.0f);
    //        break;
    //    case 2: // Metalness pass
    //        return float4(metalness, metalness, metalness, 1.0f);
    //        break;
    //    case 3: // Ambient Occlusion texture pass
    //        return float4(occlusion, occlusion, occlusion, 1.0f);
    //        break;
    //    case 4: // Ambient Occlusion screen space pass
    //        return float4(1.0f, 1.0f, 1.0f, 1.0f); // TODO : FIX
    //        break;
    //    case 5: // Normals tangent space pass
    //        return float4(1.0f, 1.0f, 1.0f, 1.0f); // TODO : FIX
    //        break;
    //    case 6: // Normals world space pass
    //        return float4(normalPixel.xyz, 1.0f);
    //        break;
    //    case 7: // Shadows pass
    //        return float4(1.0f, 1.0f, 1.0f, 1.0f); // TODO : FIX
    //        break;
    //}
    
    const float4 albedoMap = AlbedoTexture.Sample(TrilinearWrap, aPixel.UV0);
    const float2 normalMap = NormalTexture.Sample(TrilinearWrap, aPixel.UV0).rg;
    const float3 materialMap = MaterialTexture.Sample(TrilinearWrap, aPixel.UV0).rgb;
    
    float2 unpackedNormal = normalMap * 2.0 - 1.0;
    
    float3 normal = normalize(float3(unpackedNormal, sqrt(saturate(1.0 - dot(unpackedNormal, unpackedNormal)))));

    const float3x3 TBN = float3x3(
        normalize(aPixel.Tangent),
        normalize(aPixel.BiNormal),
        normalize(aPixel.Normal)
    );
    
    normal = normalize(mul(normal, TBN));
    
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
    
    result.AlbedoTexture = float4(parameters.PixelColor.rgb, 1);
    result.NormalTexture = float4(parameters.Normal, 0);
    result.MaterialTexture = float4(parameters.Occlusion, parameters.Roughness, parameters.Metalness, 0);
    result.Emission = float4(1, 1, 1, 0);
    result.WorldPosition = parameters.WorldPosition;
    
    return result;
}
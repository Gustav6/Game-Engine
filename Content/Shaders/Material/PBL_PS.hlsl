#include "Include/Common.hlsli"
#include "Include/Material.hlsli"
#include "Include/Light.hlsli"
#include "Include/Shadow.hlsli"
#include "Include/Samplers.hlsli"
#include "Include/PBL_Functions.hlsli"

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MaterialTexture : register(t2);
Texture2D EmissionTexture : register(t3);

float4 main(VStoPS aPixel) : SV_TARGET
{
    const float4 albedoMap = AlbedoTexture.Sample(TrilinearWrap, aPixel.UV0);
    const float4 normalMap = NormalTexture.Sample(TrilinearWrap, aPixel.UV0);
    const float4 materialMap = MaterialTexture.Sample(TrilinearWrap, aPixel.UV0);
    const float4 emissionMap = EmissionTexture.Sample(TrilinearWrap, aPixel.UV0);

    if (albedoMap.a < 0.05f)
    {
        discard;
    }
    
    float2 unpackedNormal = normalMap.xy * 2.0 - 1.0;
    
    float3 normal = normalize(float3(unpackedNormal, sqrt(saturate(1.0 - dot(unpackedNormal, unpackedNormal)))));

    const float3x3 TBN = float3x3(
        normalize(aPixel.Tangent),
        normalize(aPixel.BiNormal),
        normalize(aPixel.Normal)
    );

    normal = normalize(mul(normal, TBN));
    
    switch (FB_TypeOfRenderPass)
    {
        case 0: // sRGB pass do nothing!
            break;
        case 1: // Rougness pass
            return float4(materialMap.g, materialMap.g, materialMap.g, 1.0f);
        case 2: // Metalness pass
            return float4(materialMap.b, materialMap.b, materialMap.b, 1.0f);
        case 3: // Ambient Occlusion texture pass
            return float4(materialMap.r, materialMap.r, materialMap.r, 1.0f);
        case 4: // Ambient Occlusion screen space pass
            return float4(1.0f, 1.0f, 1.0f, 1.0f); // TODO : FIX
        case 5: // Normals tangent space pass
            return float4(normalMap.xy, 0.0f, 1.0f);
        case 6: // Normals world space pass
            return float4(normal, 1.0f);
        case 7: // Shadows pass
            return float4(1.0f, 1.0f, 1.0f, 1.0f); // TODO : FIX
        default:
            break;
    }
    
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
    
    const float3 specularColor = lerp((float3) 0.04f, parameters.PixelColor.rgb, parameters.Metalness);
    const float3 diffuseColor = lerp((float3) 0.00f, parameters.PixelColor.rgb, 1 - parameters.Metalness);
    const float3 pixelView = normalize(FB_CameraPosition - parameters.WorldPosition.xyz);
    
    const int envNumMips = GetNumMips(EnvCubeTexture) - 1;
    
    float3 radiance = CalculateAmbientContribution(parameters.Normal, pixelView, specularColor, diffuseColor, envNumMips, parameters.Roughness, parameters.Occlusion);
    
    for (uint l = 0; l < LB_NumActiveLights; ++l)
    {
        Light light = LB_Lights[l];
        
        switch (light.Type)
        {
            case 0:
            {
                float pixelDistance = length(parameters.WorldPosition.xyz - FB_CameraPosition);

                uint cascadeIndex;
                
                if (pixelDistance < CB_CascadeFarPlanes.x)
                {
                    //radiance = float3(1, 0, 0);
                    cascadeIndex = 0;
                }
                else if (pixelDistance < CB_CascadeFarPlanes.y)
                {
                    //radiance = float3(0, 1, 0);
                    cascadeIndex = 1;
                }
                else if (pixelDistance < CB_CascadeFarPlanes.z)
                {
                    //radiance = float3(0, 0, 1);
                    cascadeIndex = 2;
                }
                else
                {
                    //radiance = float3(1, 1, 0);
                    cascadeIndex = 3;
                }
                
                float3 lightContribution = CalculateDirectionalLightContribution(
                    light,
                    parameters.Normal,
                    pixelView,
                    specularColor,
                    diffuseColor,
                    parameters.Roughness
                );
                
                float shadowFactor = CalculateDirectionalLightShadowFactor(
                    cascadeIndex,
                    parameters.WorldPosition
                );
                
                radiance += lightContribution * shadowFactor;
                
                break;
            }
            case 1:
            {
                float3 lightContribution = CalculatePointLightContribution(
                    light,
                    parameters.Normal,
                    parameters.WorldPosition.xyz,
                    pixelView,
                    specularColor,
                    diffuseColor,
                    parameters.Roughness
                );
                
                float shadowFactor = CalculatePointLightShadowFactor(
                    light, 
                    light.ShadowIndex,
                    aPixel.WorldPosition.xyz
                );
                
                radiance += lightContribution * shadowFactor;
                
                break;
            }
            case 2:
            {
                float3 lightContribution = CalculateSpotLightContribution(
                    light,
                    parameters.Normal,
                    parameters.WorldPosition.xyz,
                    pixelView,
                    specularColor,
                    diffuseColor,
                    parameters.Roughness
                );
                
                float shadowFactor = CalculateSpotLightShadowFactor(
                    light,
                    light.ShadowIndex,
                    parameters.WorldPosition,
                    parameters.Normal
                );
               
                radiance += lightContribution * shadowFactor;
                
                break;
            }
            default:
                break;
        }
    }

    radiance += emission;
   
    return float4(radiance, parameters.PixelColor.a);
}
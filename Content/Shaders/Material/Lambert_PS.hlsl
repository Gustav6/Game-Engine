#include "Include/Common.hlsli"
#include "Include/Material.hlsli"
#include "Include/Light.hlsli"
#include "Include/Shadow.hlsli"
#include "Include/Samplers.hlsli"

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);

float4 main(VStoPS aPixel) : SV_TARGET
{
    float3 albedoPixel = AlbedoTexture.Sample(TrilinearWrap, aPixel.UV0).rgb;
    float2 normalPixel = NormalTexture.Sample(TrilinearWrap, aPixel.UV0).rg;
    
    float2 unpackedNormal = normalPixel * 2.0 - 1.0;
    
    float3 normal = normalize(float3(unpackedNormal, sqrt(saturate(1.0 - dot(unpackedNormal, unpackedNormal)))));

    const float3x3 TBN = float3x3(
        normalize(aPixel.Tangent),
        normalize(aPixel.BiNormal),
        normalize(aPixel.Normal)
    );

    normal = normalize(mul(normal, TBN));
    
    MaterialPixelParameters parameters;
    parameters.PixelColor = float4(albedoPixel, 1);
    parameters.WorldPosition = aPixel.WorldPosition;
    parameters.Normal = normal;
    parameters.UV0 = aPixel.UV0;
    
    Material_Pixel(parameters);
    
    float3 radiance = 0;
    
    for (uint l = 0; l < LB_NumActiveLights; ++l)
    {
        Light light = LB_Lights[l];
        
        switch (light.Type)
        {
            case 0:
            {
                float3 lightContribution = CalculateDirectionalLight(
                    light.Direction,
                    normal,
                    albedoPixel,
                    light.Color,
                    light.Intensity
                );
                
                float pixelDistance = length(aPixel.WorldPosition.xyz - CB_CameraPosition);

                uint cascadeIndex;
                
                if (pixelDistance < CB_CascadeFarPlanes.x)
                {
                    //lightContribution.r *= 5;
                    cascadeIndex = 0;
                }
                else if (pixelDistance < CB_CascadeFarPlanes.y)
                {
                    //lightContribution.g *= 5;
                    cascadeIndex = 1;
                }
                else if (pixelDistance < CB_CascadeFarPlanes.z)
                {
                    //lightContribution.b *= 5;
                    cascadeIndex = 2;
                }
                else
                {
                    //lightContribution.r *= 5;
                    //lightContribution.g *= 5;
                    cascadeIndex = 3;
                }
                
                float shadowFactor = CalculateDirectionalLightShadowFactor(
                    cascadeIndex,
                    CB_LightViewProjectionTextures[cascadeIndex],
                    aPixel.WorldPosition, 
                    normal, 
                    light.Direction
                );
                
                radiance += lightContribution * shadowFactor;
                
                break;
            }
            case 1:
            {
               float3 lightContribution = CalculatePointLight(
                    light.Position,
                    aPixel.WorldPosition.xyz,
                    normal,
                    albedoPixel,
                    light.Color,
                    light.Intensity,
                    light.Radius
                );
                
                float shadowFactor = CalculatePointLightShadowFactor(
                    light.ShadowIndex, 
                    aPixel.WorldPosition.xyz, 
                    light.Position, 
                    light.Radius
                );
                
                radiance += lightContribution * shadowFactor;
                
                break;
            }
            case 2:
            {
                float3 lightContribution = CalculateSpotLight(
                    light.Position,
                    light.Direction,
                    aPixel.WorldPosition.xyz,
                    normal,
                    albedoPixel,
                    light.Color,
                    light.Intensity,
                    light.Radius,
                    light.InnerCone,
                    light.OuterCone
                );
                
                float shadowFactor = CalculateSpotLightShadowFactor(
                    light.ShadowIndex, 
                    light.ShadowMapSize,
                    light.LightViewProjectionTexture, 
                    aPixel.WorldPosition, 
                    normal, 
                    light.Direction
                );
               
                radiance += lightContribution * shadowFactor;
                
                break;
            }
        }
    }
    
    float3 gammaPixel = LinearToGamma(radiance);
    
    return float4(gammaPixel, 1.0f);
}
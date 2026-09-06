#include "Include/Common.hlsli"
#include "Include/Material.hlsli"
#include "Include/Light.hlsli"
#include "Include/Shadow.hlsli"
#include "Include/Samplers.hlsli"
#include "Include/Lambert_Functions.hlsli"

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);

float4 main(VStoPS aPixel) : SV_TARGET
{
    const float3 albedoMap = AlbedoTexture.Sample(TrilinearWrap, aPixel.UV0).rgb;
    const float2 normalMap = NormalTexture.Sample(TrilinearWrap, aPixel.UV0).rg;
    
    float2 unpackedNormal = normalMap * 2.0 - 1.0;
    
    float3 normal = normalize(float3(unpackedNormal, sqrt(saturate(1.0 - dot(unpackedNormal, unpackedNormal)))));

    const float3x3 TBN = float3x3(
        normalize(aPixel.Tangent),
        normalize(aPixel.BiNormal),
        normalize(aPixel.Normal)
    );

    normal = normalize(mul(normal, TBN));
    
    MaterialPixelParameters parameters;
    parameters.PixelColor = float4(albedoMap, 1);
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
                float pixelDistance = length(aPixel.WorldPosition.xyz - CB_CameraPosition);

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
                
                float3 lightContribution = CalculateDirectionalLight(
                    light,
                    normal,
                    albedoMap
                );
                
                float shadowFactor = CalculateDirectionalLightShadowFactor(
                    cascadeIndex,
                    aPixel.WorldPosition
                );
                
                radiance += lightContribution * shadowFactor;
                
                break;
            }
            case 1:
            {
               float3 lightContribution = CalculatePointLight(
                    light,
                    aPixel.WorldPosition.xyz,
                    normal,
                    albedoMap
                );
                
                float shadowFactor = CalculatePointLightShadowFactor(
                    light, 
                    aPixel.WorldPosition.xyz
                );
                
                radiance += lightContribution * shadowFactor;
                
                break;
            }
            case 2:
            {
                float3 lightContribution = CalculateSpotLight(
                    light,
                    aPixel.WorldPosition.xyz,
                    normal,
                    albedoMap
                );
                
                float shadowFactor = CalculateSpotLightShadowFactor(
                    light, 
                    aPixel.WorldPosition, 
                    normal
                );
               
                radiance += lightContribution * shadowFactor;
                
                break;
            }
        }
    }
    
    float3 gammaPixel = LinearToGamma(radiance);
    
    return float4(gammaPixel, 1.0f);
}
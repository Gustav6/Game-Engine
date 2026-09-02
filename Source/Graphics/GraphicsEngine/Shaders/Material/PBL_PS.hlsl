#include "Include/Common.hlsli"
#include "Include/Material.hlsli"
#include "Include/Light.hlsli"
#include "Include/Shadow.hlsli"
#include "Include/Samplers.hlsli"

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MaterialTexture : register(t2);

float3 NormalDistributionFunction_GGX(float aNdotH, float aRoughness)
{
    float aRoughnessSquared = pow(aRoughness, 4);
    
    return aRoughnessSquared / (PI * pow(aNdotH * aNdotH * (aRoughnessSquared - 1) + 1, 2));
}

float3 Fresnel_SphericalGaussianSchlick(float aVdotH, float3 aSpecularColor)
{
    float3 sphericalGaussianApproximation = (-5.55473f * aVdotH - 6.98316f) * aVdotH;
    
    return aSpecularColor + (1 - aSpecularColor) * pow(2, sphericalGaussianApproximation);
}

float3 GeometricAttenuation_Smith(float aDotProduct, float aRoughness)
{
    float pixarRoughnessRemap = pow(aRoughness + 1, 2) / 8;
    
    return aDotProduct / (aDotProduct * (1 - pixarRoughnessRemap) + pixarRoughnessRemap);
}

float3 GeometricAttenuation_Schlick_GGX(float aNDotL, float aNdotV, float aRoughness)
{
    return GeometricAttenuation_Smith(aNDotL, aRoughness) * GeometricAttenuation_Smith(aNdotV, aRoughness);
}

float3 Specular_BRDF(float aRoughness, float3 aNormal, float3 aHalfAngleToCamera, float3 aPixelToCamera, float3 aPixelToLight, float3 aSpecularColor)
{
    float NdotL = saturate(dot(aNormal, aPixelToLight));
    float NdotV = saturate(dot(aNormal, aPixelToCamera));
    float NdotH = saturate(dot(aNormal, aHalfAngleToCamera));
    float VdotH = saturate(dot(aPixelToLight, aHalfAngleToCamera));
    
    return 
    (
        NormalDistributionFunction_GGX(NdotH, aRoughness) *
        Fresnel_SphericalGaussianSchlick(VdotH, aSpecularColor) *
        GeometricAttenuation_Schlick_GGX(NdotL, NdotV, aRoughness)
    ) / (4 * NdotL * NdotV + 0.00001f);
}

float3 Diffuse_BRDF(float3 aDiffuseColor)
{
    return aDiffuseColor / PI;
}

float4 main(VStoPS aPixel) : SV_TARGET
{
    const float3 albedoMap = AlbedoTexture.Sample(TrilinearWrap, aPixel.UV0).rgb;
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
    parameters.PixelColor = float4(albedoMap, 1);
    parameters.WorldPosition = aPixel.WorldPosition;
    parameters.Normal = normal;
    parameters.UV0 = aPixel.UV0;
    
    Material_Pixel(parameters);
    
    const float occlusionMap = materialMap.r;
    const float roughMap = materialMap.g;
    const float metalMap = materialMap.b;
    
    const float3 specularColor = lerp((float3) 0.04f, albedoMap.rgb, metalMap);
    const float3 diffuseColor = lerp((float3) 0.00f, albedoMap.rgb, 1 - metalMap);
    
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
                    cascadeIndex = 0;
                }
                else if (pixelDistance < CB_CascadeFarPlanes.y)
                {
                    cascadeIndex = 1;
                }
                else if (pixelDistance < CB_CascadeFarPlanes.z)
                {
                    cascadeIndex = 2;
                }
                else
                {
                    cascadeIndex = 3;
                }
                
                float3 pixelToCamera = normalize(FB_CameraPosition - aPixel.WorldPosition.xyz);
                float3 pixelToLight = normalize(-light.Direction);
                float3 halfAngleToCamera = (pixelToLight + pixelToCamera) / length(pixelToLight + pixelToCamera);
                
                float NdotL = saturate(dot(normal, pixelToLight));
    
                const float3 directLightSpecular = Specular_BRDF(roughMap, normal, halfAngleToCamera, pixelToCamera, pixelToLight, specularColor);
                const float3 directLightDiffuse = Diffuse_BRDF(diffuseColor) * (1.0f - directLightSpecular) * NdotL;
                
                float3 lightContribution = (directLightDiffuse + directLightSpecular) * light.Color * light.Intensity;
                
                float shadowFactor = CalculateDirectionalLightShadowFactor(
                    cascadeIndex,
                    CB_LightViewProjectionTextures[cascadeIndex],
                    aPixel.WorldPosition,
                    normal,
                    light.Direction
                );
                
                radiance += lightContribution;
                
                break;
                }
            case 1:
            {
                //float3 lightContribution = CalculatePointLight(
                //    light.Position,
                //    aPixel.WorldPosition.xyz,
                //    normal,
                //    albedoMap,
                //    light.Color,
                //    light.Intensity,
                //    light.Radius
                //);
                
                //float shadowFactor = CalculatePointLightShadowFactor(
                //    light.ShadowIndex,
                //    aPixel.WorldPosition.xyz,
                //    light.Position,
                //    light.Radius
                //);
                
                //radiance += lightContribution * shadowFactor;
                
                break;
            }
            case 2:
            {
                //float3 lightContribution = CalculateSpotLight(
                //    light.Position,
                //    light.Direction,
                //    aPixel.WorldPosition.xyz,
                //    normal,
                //    albedoMap,
                //    light.Color,
                //    light.Intensity,
                //    light.Radius,
                //    light.InnerCone,
                //    light.OuterCone
                //);
                
                //float shadowFactor = CalculateSpotLightShadowFactor(
                //    light.ShadowIndex,
                //    light.ShadowMapSize,
                //    light.LightViewProjectionTexture,
                //    aPixel.WorldPosition,
                //    normal,
                //    light.Direction
                //);
               
                //radiance += lightContribution * shadowFactor;
                
                break;
            }
        }
    }
    
    float3 gammaPixel = LinearToGamma(radiance);
    
    return float4(gammaPixel, 1.0f);
}
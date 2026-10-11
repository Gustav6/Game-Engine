#ifndef PBL_HLSLI
#define PBL_HLSLI

#include "Common.hlsli"
#include "Samplers.hlsli"
#include "Light.hlsli"

TextureCube EnvCubeTexture : register(t98);
Texture2D BRDF_LUT_Texture : register(t99);

int GetNumMips(TextureCube aTextureCube)
{
    int width = 0;
    int height = 0;
    int numberOfMips = 0;
    
    aTextureCube.GetDimensions(0, width, height, numberOfMips);
    
    return numberOfMips;
}

float3 NormalDistributionFunction_GGX(float aNdotH, float aRoughness)
{
    float aRoughnessSquared = pow(aRoughness, 4);
    
    return aRoughnessSquared / (PI * pow((aNdotH * aNdotH * (aRoughnessSquared - 1) + 1), 2));
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

float3 Diffuse_IBL(float3 aNormal, float aEnvNumMips)
{
    return EnvCubeTexture.SampleLevel(TrilinearWrap, aNormal, aEnvNumMips).rgb;
}

float3 Specular_IBL(float3 aNormal, float3 aPixelView, float3 aSpecularColor, float aEnvNumMips, float aRoughness)
{
    const float3 R = reflect(-aPixelView, aNormal);
    
    const float3 envColor = EnvCubeTexture.SampleLevel(TrilinearWrap, R, aRoughness * aEnvNumMips).rgb;
                
    float NdotV = saturate(dot(aNormal, aPixelView));
    const float2 brdfLUT = BRDF_LUT_Texture.Sample(LUTSampler, float2(NdotV, aRoughness)).rg;
       
    return envColor * (aSpecularColor * brdfLUT.x + brdfLUT.y);
}

float3 CalculateAmbientContribution(float3 aNormal, float3 aPixelView, float3 aSpecularColor, float3 aDiffuseColor, float aEnvNumMips, float aRoughness, float aOcclusion)
{
    const float3 diffuseIBL = Diffuse_IBL(aNormal, aEnvNumMips);
    const float3 specularIBL = Specular_IBL(aNormal, aPixelView, aSpecularColor, aEnvNumMips, aRoughness);
    
    return (aDiffuseColor * diffuseIBL + specularIBL) * aOcclusion;
}

float3 CalculateDirectionalLightContribution(in Light aDirectionalLight, float3 aNormal, float3 aPixelView, float3 aSpecularColor, float3 aDiffuseColor, float aRoughness)
{
    float3 pixelToLight = normalize(-aDirectionalLight.Direction);
    float3 halfAngleToCamera = normalize(pixelToLight + aPixelView);
                
    float NdotL = saturate(dot(aNormal, pixelToLight));
    
    const float3 specularBRDF = Specular_BRDF(aRoughness, aNormal, halfAngleToCamera, aPixelView, pixelToLight, aSpecularColor) * NdotL;
    const float3 diffuseBRDF = Diffuse_BRDF(aDiffuseColor) * (1.0f - specularBRDF);
    
    float3 radiance = aDirectionalLight.Color * aDirectionalLight.Intensity;
    
    return (diffuseBRDF + specularBRDF) * radiance;
}

float3 CalculatePointLightContribution(in Light aPointLight, float3 aNormal, float3 aPixelPosition, float3 aPixelView, float3 aSpecularColor, float3 aDiffuseColor, float aRoughness)
{
    float3 pixelToLight = normalize(aPointLight.Position - aPixelPosition);
    float distance = length(aPointLight.Position - aPixelPosition);
    
    float alpha = saturate(1.0f - pow(abs(distance / aPointLight.Radius), 2.0f));
    alpha = alpha * alpha;
    
    float attenuation = (1.0 / (distance * distance + 0.0001f)) * alpha;;
    float3 radiance = aPointLight.Color * aPointLight.Intensity * attenuation;
    
    float3 halfAngleToCamera = normalize(pixelToLight + aPixelView);

    float NdotL = saturate(dot(aNormal, pixelToLight));
    
    const float3 specularBRDF = Specular_BRDF(aRoughness, aNormal, halfAngleToCamera, aPixelView, pixelToLight, aSpecularColor) * NdotL;
    const float3 diffuseBRDF = Diffuse_BRDF(aDiffuseColor) * (1.0f - specularBRDF);
    
    return (diffuseBRDF + specularBRDF) * radiance;
}

float3 CalculateSpotLightContribution(in Light aSpotLight, float3 aNormal, float3 aPixelPosition, float3 aPixelView, float3 aSpecularColor, float3 aDiffuseColor, float aRoughness)
{
    float3 pixelToLight = normalize(aSpotLight.Position - aPixelPosition);
    float distance = length(aSpotLight.Position - aPixelPosition);
    
    float spotFactor = saturate(dot(-pixelToLight, aSpotLight.Direction));
    
    float IS = smoothstep(aSpotLight.CosOuterCone, aSpotLight.CosInnerCone, spotFactor);
    
    float alpha = saturate(1.0f - pow(abs(distance / aSpotLight.Radius), 2.0f));
    alpha = alpha * alpha * IS;
    
    float attenuation = (1.0 / (distance * distance + 0.0001f)) * alpha;;
    float3 radiance = aSpotLight.Color * aSpotLight.Intensity * attenuation;
    
    float3 halfAngleToCamera = normalize(pixelToLight + aPixelView);

    float NdotL = saturate(dot(aNormal, pixelToLight));
    
    const float3 specularBRDF = Specular_BRDF(aRoughness, aNormal, halfAngleToCamera, aPixelView, pixelToLight, aSpecularColor) * NdotL;
    const float3 diffuseBRDF = Diffuse_BRDF(aDiffuseColor) * (1.0f - specularBRDF);
    
    return (diffuseBRDF + specularBRDF) * radiance;
}

#endif
#include "Include/Common.hlsli"
#include "Include/Material.hlsli"
#include "Include/Light.hlsli"
#include "Include/Shadow.hlsli"
#include "Include/Samplers.hlsli"

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MaterialTexture : register(t2);

TextureCube EnvCubeTexture : register(t98);
Texture2D BRDF_LUT_Texture : register(t99);

int GetNumMips(TextureCube aCube)
{
    int w = 0;
    int h = 0;
    int m = 0;
    
    aCube.GetDimensions(0, w, h, m);
    
    return m;
}

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

float3 Diffuse_IBL(float3 aNormal, float aNumMips)
{
    return EnvCubeTexture.SampleLevel(TrilinearWrap, aNormal, aNumMips).rgb;
}

float3 Specular_IBL(float3 aNormal, float3 aPixelView, float3 aSpecularColor, float aNumMips, float aRoughness)
{
    const float3 R = reflect(-aPixelView, aNormal);
               
    const float3 envColor = EnvCubeTexture.SampleLevel(TrilinearWrap, R, aRoughness * aNumMips).rgb;
                
    float NdotV = saturate(dot(aNormal, aPixelView));
    const float2 brdfLUT = BRDF_LUT_Texture.Sample(LUTSampler, float2(NdotV, aRoughness)).rg;
       
    return envColor * (aSpecularColor * brdfLUT.x + brdfLUT.y);
}

float3 CalculateDirectionalLightContribution(Light aDirectionLight, float3 aNormal, float3 aPixelView, float3 aSpecularColor, float3 aDiffuseColor, float aRoughness)
{
    float3 pixelToLight = normalize(-aDirectionLight.Direction);
    float3 halfAngleToCamera = normalize(pixelToLight + aPixelView);
                
    float NdotL = saturate(dot(aNormal, pixelToLight));
    
    const float3 specularBRDF = Specular_BRDF(aRoughness, aNormal, halfAngleToCamera, aPixelView, pixelToLight, aSpecularColor);
    const float3 diffuseBRDF = Diffuse_BRDF(aDiffuseColor) * (1.0f - specularBRDF) * NdotL;
    
    return (diffuseBRDF + specularBRDF) * aDirectionLight.Color * aDirectionLight.Intensity;
}

float3 CalculateAmbientContribution(float3 aNormal, float3 aPixelView, float3 aSpecularColor, float3 aDiffuseColor, float aNumMips, float aRoughness, float aOcclusion)
{
    const float3 diffuseIBL = Diffuse_IBL(aNormal, aNumMips);
    const float3 specularIBL = Specular_IBL(aNormal, aPixelView, aSpecularColor, aNumMips, aRoughness);
    
    return (aDiffuseColor * diffuseIBL + specularIBL) * aOcclusion;
}

float3 CalculatePointLightContribution()
{
    return float3(0, 0, 0);
}

float3 CalculateSpotLightContribution()
{
    return float3(0, 0, 0);
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
    const float roughness = materialMap.g;
    const float metalness = materialMap.b;
    
    const float3 specularColor = lerp((float3) 0.04f, albedoMap.rgb, metalness);
    const float3 diffuseColor = lerp((float3) 0.00f, albedoMap.rgb, 1 - metalness);
    const float3 pixelView = normalize(FB_CameraPosition - aPixel.WorldPosition.xyz);
    
    const int numMips = GetNumMips(EnvCubeTexture) - 1;
    
    float3 radiance = CalculateAmbientContribution(normal, pixelView, specularColor, diffuseColor, numMips, roughness, occlusionMap);

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
                
                float3 lightContribution = CalculateDirectionalLightContribution(
                    light, 
                    normal, 
                    pixelView, 
                    specularColor, 
                    diffuseColor, 
                    roughness
                );
                
                float shadowFactor = CalculateDirectionalLightShadowFactor(
                    cascadeIndex,
                    CB_LightViewProjectionTextures[cascadeIndex],
                    aPixel.WorldPosition
                );
                
                radiance += lightContribution * shadowFactor;
                
                break;
            }
            case 1:
            {
                float3 lightContribution = CalculatePointLightContribution();
                
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
                float3 lightContribution = CalculateSpotLightContribution();
                
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
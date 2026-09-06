#ifndef LAMBERT_HLSLI
#define LAMBERT_HLSLI

#include "Common.hlsli"
#include "Light.hlsli"

float3 CalculateDirectionalLight(in Light aDirectionalLight, float3 aNormal, float3 aSurfaceColor)
{
    float3 L = normalize(-aDirectionalLight.Direction);
    
    float3 N = normalize(aNormal);
    
    float NdotL = saturate(dot(L, N));
    
    return NdotL * (aSurfaceColor / PI) * (aDirectionalLight.Color * aDirectionalLight.Intensity);
}

float3 CalculatePointLight(in Light aPointLight, float3 aPixelPosition, float3 aNormal, float3 aSurfaceColor)
{
    float3 lightDelta = aPointLight.Position - aPixelPosition;
    float DL = max(length(lightDelta), 0.01f);
    
    float alpha = saturate(1.0f - pow(abs(DL / aPointLight.Radius), 2.0f));
    alpha = alpha * alpha;
    
    float IL = (aPointLight.Intensity / (DL * DL)) * alpha;
    
    float3 L = normalize(lightDelta);
    float3 N = normalize(aNormal);
    
    float NdotL = saturate(dot(L, N));
    
    return NdotL * (aSurfaceColor / PI) * (aPointLight.Color * IL);
}

float3 CalculateSpotLight(in Light aSpotLight, float3 aPixelPosition, float3 aNormal, float3 aSurfaceColor)
{
    float3 lightDelta = aSpotLight.Position - aPixelPosition;
    float DL = max(length(lightDelta), 0.01f);
    
    float3 L = normalize(lightDelta);
    
    float spotFactor = saturate(dot(-L, aSpotLight.Direction));
    
    float cosInnerCone = cos(aSpotLight.InnerCone);
    float cosOuterCone = cos(aSpotLight.OuterCone);
    
    float IS = smoothstep(cosOuterCone, cosInnerCone, spotFactor);
    
    float alpha = saturate(1.0f - pow(abs(DL / aSpotLight.Radius), 2.0f));
    alpha = alpha * alpha * IS;
    
    float IL = (aSpotLight.Intensity / (DL * DL)) * alpha;
    
    float3 N = normalize(aNormal);
    float NdotL = saturate(dot(L, N));
    
    return NdotL * (aSurfaceColor / PI) * (aSpotLight.Color * IL);
}

#endif
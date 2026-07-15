#ifndef LIGHTING_HLSLI
#define LIGHTING_HLSLI

struct Light
{
    float3 Color;
    float Intensity;
    float3 Position;
    uint Type;
    float3 Direction;
    float InnerCone;
    float OuterCone;
    float Radius;
    float2 _padding;
    float4x4 LightViewProjectionTexture;
};

cbuffer LightBuffer : register(b3)
{
    Light LB_Lights[8];
    uint LB_NumActiveLights;
    float3 _LB_padding;
}

Texture2D ShadowMaps[8] : register(t100);
TextureCube PointLightShadowMap : register(t108);

float3 LinearToGamma(float3 aColor)
{
    return pow(abs(aColor), 1.0f / 2.2f);
}

float3 CalculateDirectionalLight(float3 aWorldLightDirection, float3 aWorldNormal, float3 aSurfaceColor, float3 aLightColor, float aLightIntensity)
{
    float3 L = normalize(-aWorldLightDirection);
    
    float3 N = normalize(aWorldNormal);
    
    float NdotL = saturate(dot(L, N));
    
    return NdotL * (aSurfaceColor / 3.14159265359) * (aLightColor * aLightIntensity);
}

float3 CalculatePointLight(float3 aLightPosition, float3 aPixelPosition, float3 aWorldNormal, float3 aSurfaceColor, float3 aLightColor, float aIntensity, float aRadius)
{
    float3 lightDelta = aLightPosition - aPixelPosition;
    float DL = max(length(lightDelta), 0.01f);
    
    float alpha = saturate(1.0f - pow(abs(DL / aRadius), 2.0f));
    alpha = alpha * alpha;
    
    float IL = (aIntensity / (DL * DL)) * alpha;
    
    float3 L = normalize(lightDelta);
    float3 N = normalize(aWorldNormal);
    
    float NdotL = saturate(dot(L, N));
    
    return NdotL * (aSurfaceColor / 3.14159265359) * (aLightColor * IL);
}

float3 CalculateSpotLight(float3 aLightPosition, float3 aLightDirection, float3 aPixelPosition, float3 aWorldNormal, float3 aSurfaceColor, float3 aLightColor, float aIntensity, float aRadius, float aInnerCone, float aOuterCone)
{
    float3 lightDelta = aLightPosition - aPixelPosition;
    float DL = max(length(lightDelta), 0.01f);
    
    float3 L = normalize(lightDelta);
    
    float spotFactor = saturate(dot(-L, aLightDirection));
    
    float cosInnerCone = cos(aInnerCone);
    float cosOuterCone = cos(aOuterCone);
    
    float IS = smoothstep(cosOuterCone, cosInnerCone, spotFactor);
    
    float alpha = saturate(1.0f - pow(abs(DL / aRadius), 2.0f));
    alpha = alpha * alpha * IS;
    
    float IL = (aIntensity / (DL * DL)) * alpha;
    
    float3 N = normalize(aWorldNormal);
    float NdotL = saturate(dot(L, N));
    
    return NdotL * (aSurfaceColor / 3.14159265359) * (aLightColor * IL);
}

#endif
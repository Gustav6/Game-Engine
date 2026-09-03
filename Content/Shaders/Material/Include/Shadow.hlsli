#ifndef SHADOW_HLSLI
#define SHADOW_HLSLI

#include "Light.hlsli"
#include "Samplers.hlsli"
#include "PoissonDisks.hlsli"

#define MAX_CASCADES 4

cbuffer CascadeBuffer : register(b4)
{
    float4x4 CB_LightViewProjectionTextures[MAX_CASCADES];
    float4 CB_CascadeFarPlanes;
    float4 CB_CascadeShadowMapSizes;
    float3 CB_CameraPosition;
    float _CB_Padding;
}

Texture2D DirectionalLightShadowMap[MAX_CASCADES] : register(t100);
Texture2D SpotLightShadowMaps[MAX_FORWARD_LIGHTS] : register(t104);
TextureCube PointLightShadowMaps[MAX_FORWARD_LIGHTS] : register(t112);

float SampleSpotLightShadow(uint aIndex, float2 aShadowUV, float aShadowDepth)
{
    switch (aIndex)
    {
        case 0:
            return SpotLightShadowMaps[0].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        case 1:
            return SpotLightShadowMaps[1].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        case 2:
            return SpotLightShadowMaps[2].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        case 3:
            return SpotLightShadowMaps[3].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        case 4:
            return SpotLightShadowMaps[4].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        case 5:
            return SpotLightShadowMaps[5].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        case 6:
            return SpotLightShadowMaps[6].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        case 7:
            return SpotLightShadowMaps[7].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        default:
            return 1;
    }
}

float SamplePointLightShadow(uint aIndex, float3 aShadowDirection, float aShadowDepth)
{
    switch (aIndex)
    {
        case 0:
            return PointLightShadowMaps[0].SampleCmpLevelZero(ShadowCmpSampler, aShadowDirection, aShadowDepth);
        case 1:
            return PointLightShadowMaps[1].SampleCmpLevelZero(ShadowCmpSampler, aShadowDirection, aShadowDepth);
        case 2:
            return PointLightShadowMaps[2].SampleCmpLevelZero(ShadowCmpSampler, aShadowDirection, aShadowDepth);
        case 3:
            return PointLightShadowMaps[3].SampleCmpLevelZero(ShadowCmpSampler, aShadowDirection, aShadowDepth);
        case 4:
            return PointLightShadowMaps[4].SampleCmpLevelZero(ShadowCmpSampler, aShadowDirection, aShadowDepth);
        case 5:
            return PointLightShadowMaps[5].SampleCmpLevelZero(ShadowCmpSampler, aShadowDirection, aShadowDepth);
        case 6:
            return PointLightShadowMaps[6].SampleCmpLevelZero(ShadowCmpSampler, aShadowDirection, aShadowDepth);
        case 7:
            return PointLightShadowMaps[7].SampleCmpLevelZero(ShadowCmpSampler, aShadowDirection, aShadowDepth);
        default:
            return 1;
    }
}

float SampleDirectionalLightShadow(uint aIndex, float2 aShadowUV, float aShadowDepth)
{
    switch (aIndex)
    {
        case 0:
            return DirectionalLightShadowMap[0].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        case 1:
            return DirectionalLightShadowMap[1].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        case 2:
            return DirectionalLightShadowMap[2].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        case 3:
            return DirectionalLightShadowMap[3].SampleCmpLevelZero(ShadowCmpSampler, aShadowUV, aShadowDepth);
        default:
            return 1;
    }
}

float GetCascadeShadowMapSize(uint aCascadeIndex)
{
    switch (aCascadeIndex)
    {
        case 0:
            return CB_CascadeShadowMapSizes.x;
        case 1:
            return CB_CascadeShadowMapSizes.y;
        case 2:
            return CB_CascadeShadowMapSizes.z;
        case 3:
            return CB_CascadeShadowMapSizes.w;
        default:
            return 1;
    }
}

float CalculateDirectionalLightShadowFactor(uint aCascadeIndex, float4x4 aLightViewProjectionTexture, float4 aWorldPosition)
{
    float4 lightSpacePosition = mul(aLightViewProjectionTexture, aWorldPosition);
    float3 convertedPosition = lightSpacePosition.xyz / lightSpacePosition.w;
    float2 shadowUV = convertedPosition.xy;
    float P = convertedPosition.z;
    
    float shadowTexelSize = 1.0f / GetCascadeShadowMapSize(aCascadeIndex);
    
    float shadowFactor = 0;
    
    for (int i = 0; i < 16; ++i)
    {
        float2 offset = poissonDisk16[i] * shadowTexelSize;
        shadowFactor += SampleDirectionalLightShadow(aCascadeIndex, shadowUV + offset, P);
    }
    
    return shadowFactor / 16;
}

float CalculateSpotLightShadowFactor(uint aShadowIndex, uint2 aShadowMapSize, float4x4 aLightViewProjectionTexture, float4 aWorldPosition, float3 aNormal, float3 aLightDirection)
{
    float4 lightSpacePosition = mul(aLightViewProjectionTexture, aWorldPosition);
    float3 convertedPosition = lightSpacePosition.xyz / lightSpacePosition.w;
    float2 shadowUV = convertedPosition.xy;
    float P = convertedPosition.z;
    
    float shadowTexelSize = 1.0f / aShadowMapSize.x;
    
    float shadowFactor = 0;
    
    for (int i = 0; i < 16; ++i)
    {
        float2 offset = poissonDisk16[i] * shadowTexelSize;
        shadowFactor += SampleSpotLightShadow(aShadowIndex, shadowUV + offset, P);
    }

    return shadowFactor / 16;
}

float CalculatePointLightShadowFactor(uint aShadowIndex, float3 aWorldPosition, float3 aLightPosition, float aLightRadius)
{
    const float near = 1.0f;
    const float far = aLightRadius;

    const float3 toPixel = aWorldPosition.xyz - aLightPosition;
    const float3 toLightDir = normalize(toPixel);
    
    const float3 toPixelAbs = abs(toPixel);
    const float Z = max(toPixelAbs.x, max(toPixelAbs.y, toPixelAbs.z));
    
    const float fRange = far / (far - near);
    const float v = -fRange * near;
    
    float D = (fRange * Z + v) / Z;
    
    float shadowFactor = SamplePointLightShadow(aShadowIndex, toLightDir, D);
    
    return shadowFactor;
}

#endif
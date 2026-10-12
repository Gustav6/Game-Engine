#ifndef LIGHTING_HLSLI
#define LIGHTING_HLSLI

#define MAX_FORWARD_LIGHTS 8

struct Light
{
    float3 Color;
    float Intensity;
    float3 Position;
    uint Type;
    float3 Direction;
    float CosInnerCone;
    float CosOuterCone;
    float Radius;
    uint2 ShadowMapSize;
    uint ShadowIndex;
    float _padding1;
    float _padding2;
    float _padding3;
    float4x4 LightViewProjectionTexture;
};

cbuffer LightBuffer : register(b3)
{
    Light LB_Lights[MAX_FORWARD_LIGHTS];
    uint LB_NumActiveLights;
    float3 _LB_padding;
}

#endif
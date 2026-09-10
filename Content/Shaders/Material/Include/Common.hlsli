#ifndef COMMON_HLSLI
#define COMMON_HLSLI

static const float PI = 3.14159265358979323846f;

struct VStoPS
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : WORLDPOS;
    float4 Color : COLOR;
    float2 UV0 : UV0;
    float2 UV1 : UV1;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiNormal : BINORMAL;
};

struct FullTextureVertex
{
    float4 Position : SV_POSITION;
    float2 UV : UV0;
};

cbuffer FrameBuffer : register(b0)
{
    float4x4 FB_View;
    float4x4 FB_Projection;
    float3 FB_CameraPosition;
    uint FB_TypeOfRenderPass;
}

cbuffer ObjectBuffer : register(b1)
{
    float4x4 OB_World;
    float4x4 OB_WorldInvT;
    bool OB_HasSkinning;
    float3 _OB_Padding;
}

cbuffer AnimationBuffer : register(b2)
{
    float4x4 AB_JointTransforms[128];
}

#endif
#ifndef COMMON_HLSLI
#define COMMON_HLSLI

static const float PI = 3.14159265358979323846f;

struct Vertex
{
    float4 Position : POSITION;
    float4 Color : COLOR;
    uint4 BoneIDs : BONEIDS;
    float4 SkinWeights : SKINWEIGHTS;
    float2 UV0 : UV0;
    float2 UV1 : UV1;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct LineVertex
{
    float4 Position : POSITION;
    float4 Color : COLOR;
};

struct ParticleVertex
{
    float4 Position : POSITION;
    float4 Color : COLOR;
    float3 Velocity : VELOCITY;
    float LifeTime : LIFETIME;
    float Angle : ANGLE;
    float2 Size : SIZE;
    float4 ChannelMask : CHANNELMASK;
};

struct GBufferOutput
{
    float4 Albedo : SV_TARGET0;
    float4 WolrdNormalDepth : SV_TARGET1;
    float4 Material : SV_TARGET2;
    float4 Emission : SV_TARGET3;
    float4 WorldPosition : SV_TARGET4;
};

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
    float ViewDepth : VIEWDEPTH;
};  

struct FullscreenVertex
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
    uint FB_TypeOfTonemap;
    uint FB_SSAOFlag;
    float2 _FB_Padding;
}

cbuffer ObjectBuffer : register(b1)
{
    float4x4 OB_World;
    float4x4 OB_WorldInvT;
    uint OB_SkinningFlag;
    float OB_ObjectId;
    float2 _OB_Padding;
}

cbuffer AnimationBuffer : register(b2)
{
    float4x4 AB_JointTransforms[128];
}

#endif
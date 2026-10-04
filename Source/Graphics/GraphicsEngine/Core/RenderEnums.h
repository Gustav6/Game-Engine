#pragma once

#include <cstdint>

enum class RenderMode
{
    sRGB,
    Roughness,
    Metalness,
    AmbientOcclusionTexture,
    AmbientOcclusionScreenspace,
    NormalsTangentSpace,
    NormalsWorldSpace,
    Shadows,
    MAX
};

enum class ToneMappingOperator
{
	None,
    Aces,
    Lottes,
    Unreal,
    MAX
};

enum class ConstantBuffer : uint8_t
{
    FrameBuffer,
    ObjectBuffer,
    AnimationBuffer,
    LightBuffer,
    MaterialBuffer,
    CascadeBuffer,
    MAX
};
#pragma once

#include <cstdint>

enum class DebugRenderMode
{
    None,
    sRGB,
    Roughness,
    Metalness,
    AmbientOcclusionTexture,
    AmbientOcclusionScreenSpace,
    NormalsTangentSpace,
    NormalsWorldSpace,
    Shadows,
    MAX
};

enum class ToneMapper
{
	None,
    Aces,
    Lottes,
    Unreal,
    AGX,
    AGXPunchy,
    Uncharted2,
    Uchimura,
    Filmic,
    Neutral,
    FilmicHejl,
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
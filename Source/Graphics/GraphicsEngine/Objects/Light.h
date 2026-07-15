#pragma once

#include "CommonUtilities/Matrix4x4.hpp"
#include "CommonUtilities/Vector3.hpp"
#include "CommonUtilities/Vector2.hpp"

enum class LightType : unsigned int
{
    Directional = 0,
    Point = 1,
    Spot = 2
};

struct Light
{
    CommonUtilities::Vector3f Color = { 0.0f, 0.0f, 0.0f };
    float Intensity = 0.0f;
    CommonUtilities::Vector3f Position = { 0.0f, 0.0f, 0.0f };
    LightType Type = LightType::Directional;
    CommonUtilities::Vector3f Direction = { 0.0f, 0.0f, 0.0f };
    float InnerCone = 0.0f;
    float OuterCone = 0.0f;
    float Radius = 0.0f;
    unsigned int ShadowIndex = 0;
    float _padding;
    CommonUtilities::Matrix4x4f LightViewProjectionTexture;
};
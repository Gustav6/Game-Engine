#pragma once

#include <Graphics/Objects/GPU/Texture.h>
#include <CommonUtilities/Math/Vector2.hpp>
#include <CommonUtilities/Math/Vector3.hpp>
#include <CommonUtilities/Math/Vector4.hpp>
#include <CommonUtilities/Math/Matrix4x4.hpp>

class Texture;

class ImGuiWidgets
{
public:
	static void RenderTexture(
        const Texture& aTexture,
        const CommonUtilities::Vector2ui& aSize);

    static void RenderVector2(
        const char* aLabel,
        CommonUtilities::Vector2f& aVector);

    static void RenderVector3(
        const char* aLabel,
        CommonUtilities::Vector3f& aVector);

    static void RenderVector4(
        const char* aLabel,
        CommonUtilities::Vector4f& aVector);

    static void RenderTransform(
        const char* aLabel,
        CommonUtilities::Matrix4x4f& aTransformMatrix);

    static void RenderColor(
        const char* aLabel,
        CommonUtilities::Vector4f& aColor);
};


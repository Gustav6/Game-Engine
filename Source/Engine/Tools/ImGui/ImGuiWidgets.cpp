#include "Tools/Tools.pch.h"
#include "ImGuiWidgets.h"

#include <Graphics/Objects/GPU/Texture.h>

#include <imgui/imgui.h>

void ImGuiWidgets::RenderTexture(const Texture& aTexture, const CommonUtilities::Vector2ui& aSize)
{
    ImGui::Image(
        reinterpret_cast<ImTextureID>(aTexture.GetShaderResourceHandle()),
        ImVec2(static_cast<float>(aSize.x), static_cast<float>(aSize.y))
    );
}

//void ImGuiWidgets::RenderVector2(const char* aLabel, CommonUtilities::Vector2f & aVector)
//{}
//
//void ImGuiWidgets::RenderVector3(const char* aLabel, CommonUtilities::Vector3f & aVector)
//{}
//
//void ImGuiWidgets::RenderVector4(const char* aLabel, CommonUtilities::Vector4f & aVector)
//{}
//
//void ImGuiWidgets::RenderTransform(const char* aLabel, CommonUtilities::Matrix4x4f & aTransformMatrix)
//{}
//
//void ImGuiWidgets::RenderColor(const char* aLabel, CommonUtilities::Vector4f & aColor)
//{}

#include "Tools/Tools.pch.h"
#include "SceneViewport.h"

#include <Graphics/GraphicsEngine.hpp>
#include <Graphics/Objects/GPU/Texture.h>

#include <Tools/ImGui/ImGuiWidgets.h>

#include <imgui/imgui.h>

SceneViewport::SceneViewport()
{
    myRenderTarget = std::make_unique<Texture>();

    CommonUtilities::Vector2ui clientSize = { 1920, 1080 };

    if (!GraphicsEngine::Get().CreateTexture("SceneViewportBuffer", clientSize.x, clientSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myRenderTarget))
    {
        SPDLOG_ERROR("Failed to create scene viewport texture!");
        myRenderTarget = nullptr;
        return;
    }

    auto renderOverride = std::make_unique<RenderTargetOverride>();
    renderOverride->RenderTarget = myRenderTarget.get();

    GraphicsEngine::Get().SetRenderTarget(std::move(renderOverride));
}

SceneViewport::~SceneViewport()
{
    GraphicsEngine::Get().ClearRenderTargetOverride();
}

void SceneViewport::Update(float aDeltaTime)
{
	aDeltaTime;
}

void SceneViewport::Render()
{
    if (myRenderTarget == nullptr)
    {
        return;
    }

    ImGui::Begin("Scene");

    const ImVec2 avail = ImGui::GetContentRegionAvail();

    const CommonUtilities::Vector2ui displaySize =
    {
        static_cast<unsigned int>(std::max(avail.x, 1.0f)),
        static_cast<unsigned int>(std::max(avail.y, 1.0f))
    };

    ImGuiWidgets::RenderTexture(*myRenderTarget, displaySize);
    
    ImGui::End();
}

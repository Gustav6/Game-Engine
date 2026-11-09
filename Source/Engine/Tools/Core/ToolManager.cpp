#include "Tools/Tools.pch.h"
#include "ToolManager.h"

#include <ImGui/imgui.h>

ToolManager& ToolManager::Get()
{
    static ToolManager myInstance;
    return myInstance;
}

ToolManager::ToolManager() = default;
ToolManager::~ToolManager() = default;

void ToolManager::Update(float deltaTime)
{
    for (auto& tool : myTools | std::views::values)
    {
        tool->Update(deltaTime);
    }
}

void ToolManager::Render()
{
    ImGui::PushFont(nullptr, 18.0f);

    //ImGui::ShowDemoWindow();

    for (auto& tool : myTools | std::views::values)
    {
        tool->Render();
    }

    ImGui::PopFont();
}

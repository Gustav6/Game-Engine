#include "Tools/Tools.pch.h"
#include "ToolManager.h"

#include <ImGui/imgui.h>
#include <imgui_internal.h>

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

    myToolbar.Render();

    //ImGui::ShowDemoWindow();

    for (auto& tool : myTools | std::views::values)
    {
        tool->Render();
    }

    ImGui::PopFont();
}

bool ToolManager::AddToolInternal(std::unique_ptr<ITool> aTool, std::type_index aTypeIndex, DockSlot aDockSlot)
{
    if (aTool == nullptr)
    {
        return false;
    }

    myTools.emplace(aTypeIndex, std::move(aTool));

    aDockSlot;

    return true;
}
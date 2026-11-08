#pragma once

#include <memory>
#include <typeindex>
#include <type_traits>
#include <algorithm>
#include <utility>
#include <unordered_map>

#include "ToolRegistry.h"
#include <Tools/Core/Interface/Tool.h>

class ToolManager
{
public:
	static ToolManager& Get();

	~ToolManager();

    void Update(float deltaTime);
    void Render();

    // TODO: Add method to set different imgui setting such as font.
    // TODO: Add method to allow the user to set dock settings for tools.

    template<typename ToolClass>
    requires std::is_base_of_v<Tool, ToolClass>
    bool AddTool()
    {
        const std::type_index type = typeid(ToolClass);

        if (myActiveTools.contains(type))
        {
            return false;
        }

        std::unique_ptr<Tool> tool = ToolRegistry::Get().CreateTool<ToolClass>();

        if (tool == nullptr)
        {
            return false;
        }

        myActiveTools.emplace(type, std::move(tool));

        return true;
    }

    template<typename ToolClass>
    requires std::is_base_of_v<Tool, ToolClass>
    bool RemoveTool()
    {
        const std::type_index type = typeid(ToolClass);

        auto it = myActiveTools.find(type);

        if (it == myActiveTools.end())
        {
            return false;
        }

        myActiveTools.erase(it);

        return true;
    }

    template<typename ToolClass>
    requires std::is_base_of_v<Tool, ToolClass>
    ToolClass* GetTool() const
    {
        const std::type_index type = typeid(ToolClass);

        const auto it = myActiveTools.find(type);

        if (it == myActiveTools.end())
        {
            return nullptr;
        }

        return static_cast<ToolClass*>(it->second.get());
    }

private:
	ToolManager();

    std::unordered_map<std::type_index, std::unique_ptr<Tool>> myActiveTools;
};
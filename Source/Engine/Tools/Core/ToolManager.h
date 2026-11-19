#pragma once

#include <memory>
#include <typeindex>
#include <type_traits>
#include <algorithm>
#include <utility>
#include <unordered_map>

#include "ToolRegistry.h"
#include "Toolbar.h"
#include <Tools/Core/Interface/ITool.h>

// TODO: Add method to set different settings such as font.
// TODO: Add method to allow the user to set dock settings.

enum class DockSlot
{
    None,
    Center,
    Left,
    Right,
    Top,
    Bottom,
    TopLeft,
    BottomLeft,
    TopRight,
    BottomRight
};

class ToolManager
{
public:
	static ToolManager& Get();

	~ToolManager();

    void Update(float deltaTime);
    void Render();

    template<typename ToolClass>
    requires std::is_base_of_v<ITool, ToolClass>
    bool AddTool(DockSlot aDockSlot = DockSlot::None)
    {
        const std::type_index type = typeid(ToolClass);

        if (myTools.contains(type))
        {
            return false;
        }

        std::unique_ptr<ITool> tool = ToolRegistry::Get().CreateTool<ToolClass>();

        return AddToolInternal(std::move(tool), type, aDockSlot);
    }

    template<typename ToolClass>
    requires std::is_base_of_v<ITool, ToolClass>
    bool RemoveTool()
    {
        const std::type_index type = typeid(ToolClass);

        auto it = myTools.find(type);

        if (it == myTools.end())
        {
            return false;
        }

        myTools.erase(it);

        return true;
    }

    template<typename ToolClass>
    requires std::is_base_of_v<ITool, ToolClass>
    ToolClass* GetTool() const
    {
        const std::type_index type = typeid(ToolClass);

        const auto it = myTools.find(type);

        if (it == myTools.end())
        {
            return nullptr;
        }

        return static_cast<ToolClass*>(it->second.get());
    }

private:
	ToolManager();

    bool AddToolInternal(std::unique_ptr<ITool> aTool, std::type_index aTypeIndex, DockSlot aDockSlot);

    std::unordered_map<std::type_index, std::unique_ptr<ITool>> myTools;
    Toolbar myToolbar;
};
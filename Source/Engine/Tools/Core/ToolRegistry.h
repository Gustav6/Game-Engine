#pragma once

#include <type_traits>
#include <typeindex>
#include <memory>
#include <functional>
#include <unordered_map>

#include <Tools/Core/Interface/ITool.h>

// This registry holds all registered tools as well as their "factory" method.

class ToolRegistry
{
public:
	static ToolRegistry& Get();

	~ToolRegistry();

	bool Initialize();

	template<typename ToolClass>
	requires std::is_base_of_v<ITool, ToolClass>
	bool RegisterTool()
	{
		const std::type_index type = typeid(ToolClass);

		if (myTools.contains(type))
		{
			return false;
		}

		myTools.emplace(
			type,
			[]() -> std::unique_ptr<ITool>
			{
				return std::make_unique<ToolClass>();
			}
		);

		return true;
	}

	template<typename ToolClass>
	requires std::is_base_of_v<ITool, ToolClass>
	std::unique_ptr<ITool> CreateTool() const
	{
		const std::type_index type = typeid(ToolClass);

		const auto it = myTools.find(type);

		if (it == myTools.end())
		{
			return nullptr;
		}

		return it->second();
	}

	template<typename ToolClass>
	requires std::is_base_of_v<ITool, ToolClass>
	bool IsRegistered() const
	{
		const std::type_index type = typeid(ToolClass);

		return myTools.contains(type);
	}

private:
	ToolRegistry();

	std::unordered_map<std::type_index, std::function<std::unique_ptr<ITool>()>> myTools;
};
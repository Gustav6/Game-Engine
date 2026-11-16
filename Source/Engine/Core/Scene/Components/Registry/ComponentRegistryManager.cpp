#include "Core/Core.pch.h"
#include "ComponentRegistryManager.h"

#include <ranges>

#include "Core/Scene/Components/Interface/IComponent.h"

ComponentRegistryManager::ComponentRegistryManager()
{
	myIsShutDown = false;
}

ComponentRegistryManager& ComponentRegistryManager::Get()
{
	static ComponentRegistryManager instance;
	return instance;
}

void ComponentRegistryManager::Shutdown()
{
	myRegistries.clear();
	myIsShutDown = true;
}

bool ComponentRegistryManager::IsShutdown() const
{
	return myIsShutDown;
}

std::vector<IComponent*> ComponentRegistryManager::GetAllComponents(Actor* aOwner)
{
	std::vector<IComponent*> components;
	components.reserve(myRegistries.size());

	for (auto& registry : myRegistries | std::views::values)
	{
		if (IComponent* component = registry->GetAsBase(aOwner))
		{
			components.push_back(component);
		}
	}

	return components;
}

void ComponentRegistryManager::RemoveAllComponents(Actor* aOwner)
{
	for (auto& registry : myRegistries | std::views::values)
	{
		registry->Remove(aOwner);
	}
}

void ComponentRegistryManager::Update(float aDeltaTime)
{
	for (auto& registry : myRegistries | std::views::values)
	{
		registry->Update(aDeltaTime);
	}
}

void ComponentRegistryManager::Render()
{
	for (auto& registry : myRegistries | std::views::values)
	{
		registry->Render();
	}
}

void ComponentRegistryManager::Clear()
{
	for (auto& registry : myRegistries | std::views::values)
	{
		registry->Clear();
	}
}

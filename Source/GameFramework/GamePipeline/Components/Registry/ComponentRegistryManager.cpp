#include "GameFramework.pch.h"
#include "ComponentRegistryManager.h"

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

void ComponentRegistryManager::RemoveAll(Actor* aOwner)
{
	for (auto& [type, registry] : myRegistries)
	{
		registry->Remove(aOwner);
	}
}

void ComponentRegistryManager::Update(float aDeltaTime)
{
	for (auto& [type, registry] : myRegistries)
	{
		registry->Update(aDeltaTime);
	}
}

void ComponentRegistryManager::Render()
{
	for (auto& [type, registry] : myRegistries)
	{
		registry->Render();
	}
}

void ComponentRegistryManager::Clear()
{
	for (auto& [type, registry] : myRegistries)
	{
		registry->Clear();
	}
}

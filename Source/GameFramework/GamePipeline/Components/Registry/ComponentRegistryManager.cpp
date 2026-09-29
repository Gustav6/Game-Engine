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

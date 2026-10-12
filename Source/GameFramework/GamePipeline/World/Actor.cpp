#include "GameFramework.pch.h"
#include "Actor.hpp"

#include <GamePipeline/Components/Registry/ComponentRegistryManager.h>

Actor::Actor()
{
	myIsEnabled = true;
	myIsDestroyed = false;
}

Actor::~Actor()
{
	if (!ComponentRegistryManager::Get().IsShutdown()) 
	{
		ComponentRegistryManager::Get().RemoveAll(this);
	}
}

void Actor::Update(float)
{
}

void Actor::Destroy()
{
	myIsDestroyed = true;
}

bool Actor::GetIsDestroyed() const
{
	return myIsDestroyed;
}

Transform& Actor::GetTransform()
{
	return myTransform;
}

const Transform& Actor::GetTransform() const
{
	return myTransform;
}

void Actor::Enable()
{
	myIsEnabled = true;

	for (const auto& component : ComponentRegistryManager::Get().GetAll(this))
	{
		component->Enable();
	}
}

void Actor::Disable()
{
	myIsEnabled = false;

	for (const auto& component : ComponentRegistryManager::Get().GetAll(this))
	{
		component->Disable();
	}
}

bool Actor::GetIsEnabled() const
{
	return myIsEnabled;
}

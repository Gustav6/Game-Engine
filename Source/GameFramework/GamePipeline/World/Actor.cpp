#include "GameFramework.pch.h"
#include "Actor.hpp"

#include <GamePipeline/Components/Registry/ComponentRegistryManager.h>

Actor::Actor()
{
	myIsEnabled = true;
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

Transform& Actor::GetTransform()
{
	return myTransform;
}

const Transform& Actor::GetTransform() const
{
	return myTransform;
}


void Actor::SetEnabled(bool aState)
{
	myIsEnabled = aState;
}

bool Actor::GetIsEnabled() const
{
	return myIsEnabled;
}

#include "GameFramework.pch.h"
#include "Actor.hpp"

Actor::Actor()
{
	myIsEnabled = true;
}

Actor::~Actor() = default;

void Actor::Update(float aDeltaTime)
{ 
	for (auto& entry : myComponentLookup)
	{
		entry.second->Update(aDeltaTime);
	}
}

Transform& Actor::GetTransform()
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

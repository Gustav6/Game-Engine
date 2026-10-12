#include "GameFramework.pch.h"
#include "Component.h"

#include "GamePipeline/World/Actor.hpp"

Component::Component(Actor* aOwner)
{
	myOwner = aOwner;
	myIsEnabled = aOwner->GetIsEnabled();
}

Actor* Component::GetOwner() const
{
	return myOwner;
}

void Component::Enable()
{
	myIsEnabled = true;
}

void Component::Disable()
{
	myIsEnabled = false;
}

bool Component::GetIsEnabled() const
{
	return myIsEnabled;
}

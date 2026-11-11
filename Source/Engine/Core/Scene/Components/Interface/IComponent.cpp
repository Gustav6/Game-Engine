#include "Core/Core.pch.h"
#include "IComponent.h"

#include "Core/Scene/World/Actor.hpp"

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

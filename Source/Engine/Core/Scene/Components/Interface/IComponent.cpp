#include "Core/Core.pch.h"
#include "IComponent.h"

#include "Core/Scene/World/Actor.hpp"

IComponent::IComponent(Actor* aOwner)
{
	myOwner = aOwner;
	myIsEnabled = aOwner->GetIsEnabled();
}

Actor* IComponent::GetOwner() const
{
	return myOwner;
}

void IComponent::Enable()
{
	myIsEnabled = true;
}

void IComponent::Disable()
{
	myIsEnabled = false;
}

bool IComponent::GetIsEnabled() const
{
	return myIsEnabled;
}

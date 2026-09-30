#include "GameFramework.pch.h"
#include "Component.h"

Component::Component(Actor* aOwner)
{
	myOwner = aOwner;
}

Actor* Component::GetOwner() const
{
	return myOwner;
}

#include "GameFramework.pch.h"
#include "Actor.hpp"

#include <algorithm>

#include <GamePipeline/Components/Registry/ComponentRegistryManager.h>

Actor::Actor()
{
	myIsEnabled = true;
	myIsDestroyed = false;

	myName = "";

	myParent = nullptr;
}

Actor::~Actor()
{
	if (!ComponentRegistryManager::Get().IsShutdown()) 
	{
		ComponentRegistryManager::Get().RemoveAllComponents(this);
	}

	for (const auto& child : myChildren)
	{
		child->myParent = nullptr;
		child->Destroy();
	}
	myChildren.clear();

	if (myParent != nullptr)
	{
		myParent->RemoveChild(this);
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

	for (const auto& component : ComponentRegistryManager::Get().GetAllComponents(this))
	{
		component->Enable();
	}
}

void Actor::Disable()
{
	myIsEnabled = false;

	for (const auto& component : ComponentRegistryManager::Get().GetAllComponents(this))
	{
		component->Disable();
	}
}

bool Actor::GetIsEnabled() const
{
	return myIsEnabled;
}

void Actor::SetName(const std::string& aName)
{
	myName = aName;
}

const std::string& Actor::GetName() const
{
	return myName;
}

void Actor::SetParent(Actor* aParent)
{
	if (myParent == aParent)
	{
		return;
	}

	if (aParent == this)
	{
		return;
	}

	for (Actor* ancestor = aParent; ancestor != nullptr; ancestor = ancestor->myParent)
	{
		if (ancestor == this)
		{
			return;
		}
	}

	if (myParent != nullptr)
	{
		myParent->RemoveChild(this);
	}

	myParent = aParent;

	if (myParent != nullptr)
	{
		myParent->AddChild(this);
	}
}

Actor* Actor::GetParent() const
{
	return myParent;
}

void Actor::AddChild(Actor* aChild)
{
	myChildren.emplace_back(aChild);
}

const std::vector<Actor*>& Actor::GetChildren() const
{
	return myChildren;
}

void Actor::RemoveChild(Actor* aChild)
{
	myChildren.erase(std::ranges::remove(myChildren, aChild).begin(), myChildren.end());
}

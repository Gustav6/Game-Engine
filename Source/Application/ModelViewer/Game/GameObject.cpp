#include "GameObject.hpp"

GameObject::GameObject()
{
	myIsEnabled = true;
}

GameObject::~GameObject() = default;

void GameObject::Update(float aDeltaTime)
{ 
	for (auto& entry : myComponentLookup)
	{
		entry.second->Update(aDeltaTime);
	}
}

Transform& GameObject::GetTransform()
{
	return myTransform;
}

void GameObject::SetEnabled(bool aState)
{
	myIsEnabled = aState;
}

bool GameObject::GetIsEnabled() const
{
	return myIsEnabled;
}

#pragma once

#include <string>

#include "Core/Scene/Components/Registry/ComponentRegistryManager.h"

class Actor
{
public:
	Actor(unsigned int aID);
	virtual ~Actor();

	Actor(const Actor&) = delete;
	Actor& operator=(const Actor&) = delete;

	template<typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		return ComponentRegistryManager::Get().AddComponent<T>(this, std::forward<Args>(args)...);
	}

	template<typename T>
	void RemoveComponent()
	{
		ComponentRegistryManager::Get().RemoveComponent<T>(this);
	}

	template<typename T>
	T* GetComponent()
	{
		return ComponentRegistryManager::Get().GetComponent<T>(this);
	}

	template<typename T>
	const T* GetComponent() const
	{
		return ComponentRegistryManager::Get().GetComponent<T>(this);
	}
	
	virtual void Update(float aDeltaTime);

	void Destroy();
	bool GetIsDestroyed() const;

	void Enable();
	void Disable();
	bool GetIsEnabled() const;

	void SetName(const std::string& aName);
	const std::string& GetName() const;

	void SetParent(Actor* aParent, bool aKeepWorldTransform = true);
	Actor* GetParent() const;

	void AddChild(Actor* aChild);
	const std::vector<Actor*>& GetChildren() const;

	void RemoveChild(Actor* aChild);

	unsigned int GetID() const;

private:
	Actor* myParent;
	std::vector<Actor*> myChildren;

	std::string myName;

	unsigned int myID;

	bool myIsDestroyed;
	bool myIsEnabled;
};
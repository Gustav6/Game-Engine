#pragma once

#include <string>

#include "Core/Scene/Components/Concrete/Transform/Transform.h"
#include "Core/Scene/Components/Registry/ComponentRegistryManager.h"

class Actor
{
public:
	Actor();
	virtual ~Actor();

	Actor(const Actor&) = delete;
	Actor& operator=(const Actor&) = delete;

	/*
	 * @return A pointer to the component. The pointer is only valid for the relevant scope and must not be stored for later use.
	 */
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

	/*
	 * @return A pointer to the component. The pointer is only valid for the relevant scope and must not be stored for later use.
	 */
	template<typename T>
	T* GetComponent()
	{
		return ComponentRegistryManager::Get().GetComponent<T>(this);
	}
	
	virtual void Update(float aDeltaTime);

	void Destroy();
	bool GetIsDestroyed() const;

	Transform& GetTransform();
	const Transform& GetTransform() const;

	void Enable();
	void Disable();
	bool GetIsEnabled() const;

	void SetName(const std::string& aName);
	const std::string& GetName() const;

	void SetParent(Actor* aParent);
	Actor* GetParent() const;

	void AddChild(Actor* aChild);
	const std::vector<Actor*>& GetChildren() const;

	void RemoveChild(Actor* aChild);

private:
	Actor* myParent;
	std::vector<Actor*> myChildren;

	Transform myTransform;

	std::string myName;

	bool myIsDestroyed;
	bool myIsEnabled;
};
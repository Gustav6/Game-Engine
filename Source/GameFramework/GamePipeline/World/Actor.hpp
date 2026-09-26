#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

#include "GamePipeline/Components/Transform/Transform.h"
#include "GamePipeline/Components/Base/Component.h"
#include "GamePipeline/Components/Registry/ComponentRegistryManager.h"

class Actor
{
public:
	Actor();
	virtual ~Actor();

	Actor(const Actor&) = delete;
	Actor& operator=(const Actor&) = delete;

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		T& component = ComponentRegistryManager::Get().AddComponent<T>(this, std::forward<Args>(args)...);
		myComponentLookup[std::type_index(typeid(T))] = &component;
		return component;
	}

	template<typename T>
	void RemoveComponent()
	{
		auto it = myComponentLookup.find(std::type_index(typeid(T)));

		if (it == myComponentLookup.end())
		{
			return;
		}

		ComponentRegistryManager::Get().RemoveComponent<T>(this);
		myComponentLookup.erase(it);
	}

	template<typename T>
	T* GetComponent()
	{
		auto it = myComponentLookup.find(std::type_index(typeid(T)));

		if (it == myComponentLookup.end())
		{
			return nullptr;
		}

		return static_cast<T*>(it->second);
	}
	
	virtual void Update(float aDeltaTime);

	Transform& GetTransform();
	const Transform& GetTransform() const;

	void SetEnabled(bool aState);
	bool GetIsEnabled() const;

private:
	std::unordered_map<std::type_index, Component*> myComponentLookup;

	Transform myTransform;

	bool myIsEnabled;
};
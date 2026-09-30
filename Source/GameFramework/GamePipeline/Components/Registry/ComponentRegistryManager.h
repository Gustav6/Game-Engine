#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

#include "ComponentRegistry.h"

class ComponentRegistryManager
{
public:
	ComponentRegistryManager();

	static ComponentRegistryManager& Get(); 
	
	void Shutdown();
	bool IsShutdown() const;

	template<typename T, typename... Args>
	T* AddComponent(Actor* aOwner, Args&&... aArgs)
	{
		return GetRegistry<T>().Add(aOwner, std::forward<Args>(aArgs)...);
	}

	template<typename T>
	void RemoveComponent(Actor* aOwner)
	{
		GetRegistry<T>().Remove(aOwner);
	}

	template<typename T>
	T* GetComponent(Actor* aOwner)
	{
		return GetRegistry<T>().Get<T>(aOwner);
	}

	void RemoveAll(Actor* aOwner);

	void Update(float aDeltaTime);
	void Render();

	void Clear();

private:
	template<typename T>
	ComponentRegistry<T>& GetRegistry()
	{
		std::type_index registryType = std::type_index(typeid(T));

		auto it = myRegistries.find(registryType);

		if (it == myRegistries.end())
		{
			it = myRegistries.emplace(registryType, std::make_unique<ComponentRegistry<T>>()).first;
		}

		return static_cast<ComponentRegistry<T>&>(*it->second);
	}

	bool myIsShutDown;

	std::unordered_map<std::type_index, std::unique_ptr<BaseComponentRegistry>> myRegistries;
};


#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "ComponentRegistry.h"

class IComponent;

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
		return GetRegistry<T>().Get(aOwner);
	}

	template<typename T>
	const T* GetComponent(const Actor* aOwner) const
	{
		const ComponentRegistry<T>* registry = GetRegistry<T>();

		if (registry == nullptr)
		{
			return nullptr;
		}

		return registry->Get(aOwner);
	}

	std::vector<IComponent*> GetAllComponents(Actor* aOwner);
	void RemoveAllComponents(Actor* aOwner);

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

	template<typename T>
	const ComponentRegistry<T>* GetRegistry() const
	{
		std::type_index registryType(typeid(T));

		auto it = myRegistries.find(registryType);

		if (it == myRegistries.end())
		{
			return nullptr;
		}

		return static_cast<const ComponentRegistry<T>*>(it->second.get());
	}

	bool myIsShutDown;

	std::unordered_map<std::type_index, std::unique_ptr<IComponentRegistry>> myRegistries;
};


#pragma once

#include <vector>
#include <unordered_map>

#include "GamePipeline/Components/Base/Component.h"

#include "plfhive/plf_hive.h"

class Actor;

class BaseComponentRegistry
{
public:
	virtual ~BaseComponentRegistry() = default;

	virtual void Update(float aDeltaTime) = 0;
	virtual void Render() = 0;
	virtual void Remove(Actor* aOwner) = 0;
	virtual void Clear() = 0;
	virtual Component* GetAsBase(Actor* aOwner) = 0;
};

template<typename T>
class ComponentRegistry : public BaseComponentRegistry
{
	static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

	using Hive = plf::hive<T>;
	using Iterator = Hive::iterator;

public:
	template<typename... Args>
	T* Add(Actor* aOwner, Args&&... aArgs)
	{
		Iterator it = myComponents.emplace(aOwner, std::forward<Args>(aArgs)...);

		myOwnerToComponentLookup.emplace(aOwner, it);

		return &(*it);
	}

	void Remove(Actor* aOwner) override
	{
		auto it = myOwnerToComponentLookup.find(aOwner);

		if (it == myOwnerToComponentLookup.end())
		{
			return;
		}

		myComponents.erase(it->second);
		myOwnerToComponentLookup.erase(it);
	}

	T* Get(Actor* aOwner)
	{
		auto it = myOwnerToComponentLookup.find(aOwner);

		if (it == myOwnerToComponentLookup.end())
		{
			return nullptr;
		}

		return &(*it->second);
	}

	void Update(float aDeltaTime) override
	{
		for (T& component : myComponents)
		{
			if (component.GetIsEnabled())
			{
				component.Update(aDeltaTime);
			}
		}
	}

	void Render() override
	{
		for (T& component : myComponents)
		{
			if (component.GetIsEnabled())
			{
				component.Render();
			}
		}
	}

	void Clear() override
	{
		myOwnerToComponentLookup.clear();
		myComponents.clear();
	}

	Component* GetAsBase(Actor* aOwner) override
	{
		return Get(aOwner);
	}

private:
	std::unordered_map<Actor*, Iterator> myOwnerToComponentLookup;
	Hive myComponents;
};
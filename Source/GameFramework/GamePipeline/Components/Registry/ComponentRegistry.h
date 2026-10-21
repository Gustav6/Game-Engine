#pragma once

#include <vector>
#include <unordered_map>

#include "GamePipeline/Components/Base/Component.h"

// TODO: Change vector usages and save components in chunks!
// Source : https://github.com/mattreecebentley/plf_hive

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

public:
	template<typename... Args>
	T* Add(Actor* aOwner, Args&&... aArgs)
	{
		myComponents.emplace_back(aOwner, std::forward<Args>(aArgs)...);

		myOwnerToComponentLookup.emplace(aOwner, myComponents.size() - 1);

		return &myComponents.back();
	}

	void Remove(Actor* aOwner) override
	{
		auto it = myOwnerToComponentLookup.find(aOwner);

		if (it == myOwnerToComponentLookup.end())
		{
			return;
		}

		size_t index = it->second;

		if (index != myComponents.size() - 1)
		{
			Actor* ownerOfSwap = myComponents.back().GetOwner();
			myOwnerToComponentLookup.at(ownerOfSwap) = index;

			std::swap(myComponents[index], myComponents.back());
		}

		myComponents.pop_back();

		myOwnerToComponentLookup.erase(aOwner);
	}

	T* Get(Actor* aOwner)
	{
		auto it = myOwnerToComponentLookup.find(aOwner);

		if (it == myOwnerToComponentLookup.end())
		{
			return nullptr;
		}

		return &myComponents[it->second];
	}

	void Update(float aDeltaTime) override
	{
		for (size_t c = 0; c < myComponents.size(); c++)
		{
			if (myComponents[c].GetIsEnabled())
			{
				myComponents[c].Update(aDeltaTime);
			}
		}
	}

	void Render() override
	{
		for (size_t c = 0; c < myComponents.size(); c++)
		{
			if (myComponents[c].GetIsEnabled())
			{
				myComponents[c].Render();
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
	std::unordered_map<Actor*, size_t> myOwnerToComponentLookup;
	std::vector<T> myComponents;
};
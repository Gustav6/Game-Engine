#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "GamePipeline/Components/Base/Component.h"

// TODO: Store the components so their owner can be found via the same index.
// TODO: Find a way to avoid looking up the enabled flag for the actors.

class Actor;

class BaseComponentRegistry
{
public:
	virtual ~BaseComponentRegistry() = default;

	virtual void Update(float aDeltaTime) = 0;
	virtual void Render() = 0;
	virtual void Remove(Actor* aOwner) = 0;
	virtual void Clear() = 0;
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

	template<typename... Args>
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
		for (size_t a = 0; a < myComponents.size(); a++)
		{
			if (myComponents[a].GetOwner()->GetIsEnabled())
			{
				myComponents[a].Update(aDeltaTime);
			}
		}
	}

	void Render() override
	{
		for (size_t a = 0; a < myComponents.size(); a++)
		{
			if (myComponents[a].GetOwner()->GetIsEnabled())
			{
				myComponents[a].Render();
			}
		}
	}

	void Clear()
	{
		myOwnerToComponentLookup.clear();
		myComponents.clear();
	}

private:
	std::unordered_map<Actor*, size_t> myOwnerToComponentLookup;
	std::vector<T> myComponents;
};
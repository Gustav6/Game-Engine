#pragma once

#include <memory>
#include <vector>
#include <algorithm>

#include "GamePipeline/Components/Base/Component.h"

class Actor;

class BaseComponentRegistry
{
public:
	virtual ~BaseComponentRegistry() = default;

	virtual void Update(float aDeltaTime) = 0;
	virtual void Render() = 0;
	virtual void Remove(Actor* aOwner) = 0;
};

template<typename T>
class ComponentRegistry : public BaseComponentRegistry
{
	static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

public:
	template<typename... Args>
	T& Add(Actor* aOwner, Args&&... aArgs)
	{
		myOwners.push_back(aOwner);
		myComponents.push_back(std::make_unique<T>(aOwner, std::forward<Args>(aArgs)...));

		return *myComponents.back();
	}

	void Remove(Actor* aOwner) override
	{
		auto it = std::find(myOwners.begin(), myOwners.end(), aOwner);

		if (it == myOwners.end())
		{
			return;
		}

		size_t index = std::distance(myOwners.begin(), it);

		myOwners[index] = myOwners.back();
		myComponents[index] = std::move(myComponents.back());

		myOwners.pop_back();
		myComponents.pop_back();
	}

	void Update(float aDeltaTime) override
	{
		for (size_t a = 0; a < myComponents.size(); a++)
		{
			if (myOwners[a]->GetIsEnabled())
			{
				myComponents[a]->Update(aDeltaTime);
			}
		}
	}

	void Render() override
	{
		for (size_t a = 0; a < myComponents.size(); a++)
		{
			if (myOwners[a]->GetIsEnabled())
			{
				myComponents[a]->Render();
			}
		}
	}

private:
	std::vector<Actor*> myOwners;
	std::vector<std::unique_ptr<T>> myComponents;
};
#include "Core/Core.pch.h"
#include "GameWorld.h"

#include "Actor.hpp"

#include "Core/Scene/Components/Registry/ComponentRegistryManager.h"

GameWorld::GameWorld()
{
	myNextIDCounter = 1;
	myAvailableIDs.emplace_back(myNextIDCounter);
}

GameWorld& GameWorld::Get()
{
	static GameWorld myInstance;
	return myInstance;
}

void GameWorld::Update(float aDeltaTime)
{
	ComponentRegistryManager::Get().Update(aDeltaTime);

	for (auto& actor : myQueuedActors)
	{
		myActors.insert_or_assign(actor->GetID(), std::move(actor));
	}
	myQueuedActors.clear();

	for (auto& actor : myActors | std::views::values)
	{
		if (actor->GetIsDestroyed())
		{
			continue;
		}

		actor->Update(aDeltaTime);
	}
	
	for (auto it = myActors.begin(); it != myActors.end();)
	{
		auto& actor = it->second;

		if (actor->GetIsDestroyed())
		{
			myAvailableIDs.emplace_back(it->first);
			it = myActors.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void GameWorld::Render()
{
	ComponentRegistryManager::Get().Render();
}

void GameWorld::Clear()
{
	for (auto& actor : myActors | std::views::values)
	{
		actor->Destroy();
	}
}

const std::vector<Actor*> GameWorld::GetActors() const
{
	std::vector<Actor*> actors;
	actors.reserve(myActors.size());

	for (const auto& actor : myActors | std::views::values)
	{
		actors.emplace_back(actor.get());
	}

	return actors;
}

const Actor* GameWorld::GetActor(unsigned int aID) const
{
	return myActors.at(aID).get();
}

Actor* GameWorld::GetActor(unsigned int aID)
{
	return myActors.at(aID).get();
}
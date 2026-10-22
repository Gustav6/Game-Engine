#include "GameFramework.pch.h"
#include "GameWorld.h"

#include "Actor.hpp"
#include "GamePipeline/Components/Registry/ComponentRegistryManager.h"

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
		myActors.emplace_back(std::move(actor));
	}
	myQueuedActors.clear();

	for (const auto& actor : myActors)
	{
		if (actor->GetIsDestroyed())
		{
			continue;
		}

		actor->Update(aDeltaTime);
	}
	
	for (auto it = myActors.begin(); it != myActors.end();)
	{
		if (*it && (*it)->GetIsDestroyed())
		{
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
	for (auto& actor : myActors)
	{
		actor->Destroy();
	}
}

Actor* GameWorld::AddActor(std::unique_ptr<Actor> aActor)
{
	Actor* rawPointer = aActor.get();

	myQueuedActors.emplace_back(std::move(aActor));

	return rawPointer;
}

void GameWorld::AddActors(std::vector<std::unique_ptr<Actor>> aActors)
{
	for (auto& actor : aActors)
	{
		myQueuedActors.emplace_back(std::move(actor));
	}
}

const std::vector<std::unique_ptr<Actor>>& GameWorld::GetActors() const
{
	return myActors;
}
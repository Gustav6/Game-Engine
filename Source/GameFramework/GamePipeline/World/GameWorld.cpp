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

	for (const auto& actor : myActors)
	{
		actor->Update(aDeltaTime);
	}
}

void GameWorld::Render()
{
	ComponentRegistryManager::Get().Render();
}

Actor* GameWorld::AddActor(std::unique_ptr<Actor> aActor)
{
	Actor* rawPointer = aActor.get();

	myActors.emplace_back(std::move(aActor));

	return rawPointer;
}

void GameWorld::AddActors(std::vector<std::unique_ptr<Actor>> aActors)
{
	for (auto& actor : aActors)
	{
		myActors.emplace_back(std::move(actor));
	}
}

const std::vector<std::unique_ptr<Actor>>& GameWorld::GetActors() const
{
	return myActors;
}
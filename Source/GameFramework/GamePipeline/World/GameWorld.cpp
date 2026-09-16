#include "GameFramework.pch.h"
#include "GameWorld.h"

#include "Actor.hpp"

void GameWorld::Update(float aDeltaTime)
{
	for (auto& actor : myActors)
	{
		if (!actor->GetIsEnabled())
		{
			continue;
		}

		actor->Update(aDeltaTime);
	}
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
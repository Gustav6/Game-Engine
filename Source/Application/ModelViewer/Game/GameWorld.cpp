#include "GameWorld.h"

#include "GameObject.hpp"

GameObject* GameWorld::AddGameObject(std::unique_ptr<GameObject> aGameObject)
{
	GameObject* rawPointer = aGameObject.get();

	myObjects.emplace_back(std::move(aGameObject));

	return rawPointer;
}

void GameWorld::Update(float aDeltaTime)
{
	for (auto& gameObject : myObjects)
	{
		if (!gameObject->GetIsEnabled())
		{
			continue;
		}

		gameObject->Update(aDeltaTime);
	}
}

const std::vector<std::unique_ptr<GameObject>>& GameWorld::GetGameObjects() const
{
	return myObjects;
}
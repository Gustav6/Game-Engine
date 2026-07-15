#pragma once

#include <vector>
#include <memory>

class GameObject;

class GameWorld
{
public:
    GameObject* AddGameObject(std::unique_ptr<GameObject> aGameObject);

    void Update(float aDeltaTime);

    const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const;

private:
    std::vector<std::unique_ptr<GameObject>> myObjects;
};

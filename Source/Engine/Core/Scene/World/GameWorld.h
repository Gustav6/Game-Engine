#pragma once

#include <vector>
#include <memory>

class Actor;

class GameWorld
{
public:
    static GameWorld& Get();

    void Update(float aDeltaTime);
    void Render();

    void Clear();

    Actor* AddActor(std::unique_ptr<Actor> aActor);
    void AddActors(std::vector<std::unique_ptr<Actor>> aActors);

    const std::vector<std::unique_ptr<Actor>>& GetActors() const;

private:
    std::vector<std::unique_ptr<Actor>> myActors;
    std::vector<std::unique_ptr<Actor>> myQueuedActors;
};

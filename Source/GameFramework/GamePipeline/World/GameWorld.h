#pragma once

#include <vector>
#include <memory>

class Actor;

class GameWorld
{
public:
    void Update(float aDeltaTime);

    Actor* AddActor(std::unique_ptr<Actor> aActor);
    void AddActors(std::vector<std::unique_ptr<Actor>> aActors);

    const std::vector<std::unique_ptr<Actor>>& GetActors() const;

private:
    std::vector<std::unique_ptr<Actor>> myActors;
};

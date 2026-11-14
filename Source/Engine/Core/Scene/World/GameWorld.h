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

    template<typename ActorClass>
    requires std::is_base_of_v<Actor, ActorClass>
    ActorClass* CreateActor(const std::string& aName)
    {
        unsigned int id = myAvailableIDs.back();
        myAvailableIDs.pop_back();

        if (myAvailableIDs.empty())
        {
            myNextIDCounter++;
            myAvailableIDs.emplace_back(myNextIDCounter);
        }

        std::unique_ptr<ActorClass> actor = std::make_unique<ActorClass>(id);
        actor->SetName(aName);

        SPDLOG_INFO("Actor {}, Has ID: {}", aName, id);

        ActorClass* rawPointer = actor.get();

        myQueuedActors.emplace_back(std::move(actor));

        return rawPointer;
    }

    const std::vector<Actor*> GetActors() const;

    const Actor* GetActor(unsigned int aID) const;
    Actor* GetActor(unsigned int aID);

private:
    GameWorld();

    std::unordered_map<unsigned int, std::unique_ptr<Actor>> myActors;
    std::vector<std::unique_ptr<Actor>> myQueuedActors;

    std::vector<unsigned int> myAvailableIDs;
    unsigned int myNextIDCounter;
};

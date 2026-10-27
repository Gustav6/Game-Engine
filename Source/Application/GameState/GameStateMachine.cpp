#include "GameStateMachine.h"

#include <ranges>

#include "Wrappers/Logger/LoggerWrapper.h"

#include "States/InGameState.h"

GameStateMachine::GameStateMachine() = default;
GameStateMachine::~GameStateMachine() = default;

void GameStateMachine::Initialize()
{
    myStates.emplace(GameStateType::InGame, std::make_unique<InGameState>());

    for (const auto& state : myStates | std::views::values)
    {
        state->Initialize();
    }
}

void GameStateMachine::Update(float aDeltaTime) const
{
    if (myCurrentState != nullptr)
    {
        myCurrentState->Update(aDeltaTime);
    }
}

void GameStateMachine::Render() const
{
    if (myCurrentState != nullptr)
    {
        myCurrentState->Render();
    }
}

void GameStateMachine::ChangeState(GameStateType aGameStateType)
{
    const auto it = myStates.find(aGameStateType);
    if (it == myStates.end() || it->second == nullptr)
    {
        SPDLOG_ERROR("Tried to enter a state that doesn't exist!");
        return;
    }

    GameState* newState = it->second.get();

    if (newState == myCurrentState)
    {
        SPDLOG_WARN("Tried to renter the state that the game is in!");
        return;
    }

    if (myCurrentState != nullptr)
    {
        myCurrentState->Exit();
    }

    myCurrentState = newState;
    myCurrentState->Enter();
}
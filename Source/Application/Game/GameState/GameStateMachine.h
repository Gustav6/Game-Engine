#pragma once

#include <memory>
#include <unordered_map>

enum class GameStateType
{
	InGame,
	MainMenu,
};

class GameState
{
public:
	virtual ~GameState() = default;

	virtual void Initialize() = 0;
	virtual void Enter() = 0;
	virtual void Update(float aDeltaTime) = 0;
	virtual void Render() = 0;
	virtual void Exit() = 0;
};

class GameStateMachine
{
public:
	GameStateMachine();
	~GameStateMachine();

	void Initialize();
	void Update(float aDeltaTime) const;
	void Render() const;

	void ChangeState(GameStateType aGameStateType);

private:
	std::unordered_map<GameStateType, std::unique_ptr<GameState>> myStates;
	GameState* myCurrentState;
};
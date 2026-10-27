#pragma once

#include "GameState/GameStateMachine.h"

class MeshComponent;
class Actor;

class InGameState : public GameState
{
public:
	InGameState();

	void Initialize() override;
	void Enter() override;
	void Update(float aDeltaTime) override;
	void Render() override;
	void Exit() override;

private:
	void LoadUnrealScene(const char* aSceneName);
	void LoadTestScene();

	void UpdateTestBinds();
	void UpdateGraphicsDebugBinds();

	Actor* myAnimatedObject;
	Actor* myDirectionLightObject;
	Actor* myPointLightObject;
	Actor* mySpotLightObject;

	bool myEnableTestBinds;
};


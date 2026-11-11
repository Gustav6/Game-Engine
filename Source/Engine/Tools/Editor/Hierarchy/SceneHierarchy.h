#pragma once

#include <Tools/Core/Interface/ITool.h>

class Actor;

class SceneHierarchy : public ITool
{
public:
	void Update(float aDeltaTime) override;
	void Render() override;

private:
	void DisplayActorNode(Actor* aActor);
	void DebugPrintNodesChildren(Actor* aActor);

	Actor* mySelectedActor = nullptr;
};


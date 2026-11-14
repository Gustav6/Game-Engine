#pragma once

#include <Tools/Core/Interface/ITool.h>

//#include <ImGuizmo/ImGuizmo.h>
//#include <ImGuizmo/ImVectorEditor.h>

class Actor;

class SceneHierarchy : public ITool
{
public:
	void Update(float aDeltaTime) override;
	void Render() override;

	void SetSelectedActor(Actor* aActor);

private:
	void DisplayActorNode(Actor* aActor);
	void DebugPrintNodesChildren(Actor* aActor);

	Actor* mySelectedActor = nullptr;
};


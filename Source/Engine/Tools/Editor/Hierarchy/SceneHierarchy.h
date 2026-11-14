#pragma once

#include <Tools/Core/Interface/ITool.h>

class Actor;
struct CameraData;

class SceneHierarchy : public ITool
{
public:
	void Update(float aDeltaTime) override;
	void Render() override;

	void SetSelectedActor(Actor* aActor);
	void SetCameraReference(const CameraData* aCamera);

private:
	void DisplayActorNode(Actor* aActor);
	void DebugPrintNodesChildren(Actor* aActor);

	Actor* mySelectedActor = nullptr;
	const CameraData* myCameraReference = nullptr;
};


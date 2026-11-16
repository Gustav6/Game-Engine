#pragma once

#include <unordered_set>

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
	void LogActorsChildren(Actor* aActor);

	enum class Operation
	{
		Translate,
		Rotate,
		Scale
	};

	enum class Mode
	{
		Local,
		World
	};

	Actor* mySelectedActor = nullptr;
	const CameraData* myCameraReference = nullptr;

	Operation myCurrentOperation = Operation::Translate;
	Mode myCurrentMode = Mode::World;

	std::unordered_set<Actor*> myActorsToOpen;
};


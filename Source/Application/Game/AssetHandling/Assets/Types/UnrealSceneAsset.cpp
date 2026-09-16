#include "UnrealSceneAsset.h"

UnrealSceneAsset::~UnrealSceneAsset() = default;

std::vector<std::unique_ptr<Actor>>& UnrealSceneAsset::GetSceneActors()
{
	return myActors;
}

bool UnrealSceneAsset::Load(const std::filesystem::path&)
{
	// TODO: Load all actors form scene here

	return false;
}

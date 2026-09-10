#pragma once

#include <filesystem>
#include <vector>
#include <memory>

#include "AssetHandling/Asset/Base/Asset.h"

#include "Game/World/Actor.hpp"

class UnrealSceneAsset : public Asset
{
	friend class AssetRegistry;

public:
	~UnrealSceneAsset() override;

	//std::vector<std::unique_ptr<Actor>> GetSceneActors() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	std::vector<std::unique_ptr<Actor>> myActors;
};


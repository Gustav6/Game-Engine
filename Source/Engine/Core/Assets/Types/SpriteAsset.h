#pragma once

#include <filesystem>

#include "Core/Assets/Base/Asset.h"

struct SpriteSharedData;

class SpriteAsset : public Asset
{
	friend class AssetRegistry;

public:
	const SpriteSharedData& GetData() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	std::unique_ptr<SpriteSharedData> myData;
};


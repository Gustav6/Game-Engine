#pragma once

#include <filesystem>

#include "../Base/Asset.h"

struct SpriteShared;

class SpriteAsset : public Asset
{
	friend class AssetRegistry;

public:
	SpriteAsset();
	~SpriteAsset() override;

	const SpriteShared& GetSprite() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	std::unique_ptr<SpriteShared> mySpriteShared;

};


#pragma once

#include <filesystem>

#include "AssetHandling/Asset/Base/Asset.h"

#include "GraphicsEngine/Objects/Animation.h"

class AnimationAsset : public Asset
{
	friend class AssetRegistry;

public:
	~AnimationAsset() override;

	const Animation& GetAnimation() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	Animation myAnimation;
};


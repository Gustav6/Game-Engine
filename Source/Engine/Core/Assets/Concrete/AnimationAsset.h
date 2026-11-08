#pragma once

#include <filesystem>
#include <memory>

#include "Core/Assets/Interface/Asset.h"

struct Animation;

class AnimationAsset : public Asset
{
	friend class AssetRegistry;

public:
	AnimationAsset();
	~AnimationAsset() override;

	const Animation& GetAnimation() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	std::unique_ptr<Animation> myAnimation;
};


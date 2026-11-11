#pragma once

#include <filesystem>

#include "Core/Assets/Interface/IAsset.h"

class Texture;

class TextureAsset : public Asset
{
	friend class AssetRegistry;

public:
	~TextureAsset() override;

	const std::shared_ptr<Texture>& GetTexture() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	std::shared_ptr<Texture> myTexture;
};


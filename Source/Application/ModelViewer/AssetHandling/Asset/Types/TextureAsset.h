#pragma once

#include <filesystem>

#include "AssetHandling/Asset/Base/Asset.h"

#include "GraphicsEngine/Objects/Texture.h"

class TextureAsset : public Asset
{
	friend class AssetRegistry;

public:
	~TextureAsset() override;

	const Texture& GetTexture() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	Texture myTexture;
};


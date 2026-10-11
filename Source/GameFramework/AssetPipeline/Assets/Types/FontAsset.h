#pragma once

#include <filesystem>

#include "AssetPipeline/Assets/Base/Asset.h"

#include "GraphicsEngine/Objects/Text/Font.h"

class Texture;

class FontAsset : public Asset
{
	friend class AssetRegistry;

public:
	const Font& GetFont() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	std::unique_ptr<Font> myFont;
};
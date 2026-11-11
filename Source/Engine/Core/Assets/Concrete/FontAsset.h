#pragma once

#include <filesystem>

#include "Core/Assets/Interface/IAsset.h"

#include "Graphics/Objects/Text/Font.h"

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
#pragma once

#include <filesystem>

#include "../Base/Asset.h"

class Font;
class Texture;

class FontAsset : public Asset
{
	friend class AssetRegistry;

public:
	FontAsset();
	~FontAsset() override;

	const std::shared_ptr<Font>& GetFont() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	std::shared_ptr<Font> myFont;
};
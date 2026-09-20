#pragma once

#include <filesystem>

#include "../Base/Asset.h"

class Text;

class TextAsset : public Asset
{
	friend class AssetRegistry;

public:
	TextAsset();
	~TextAsset() override;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	std::unique_ptr<Text> myText;
};
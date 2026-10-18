#include "GameFramework.pch.h"
#include "FontAsset.h"

#include <memory>

#include "GraphicsEngine/GraphicsEngine.hpp"

#include <tracy/core/Tracy.hpp>

#include "Logger/Logger.h"

const Font& FontAsset::GetFont() const
{
	return *myFont;
}

bool FontAsset::Load(const std::filesystem::path& aPath)
{
	// Command 1: msdf-atlas-gen -varfont "FONTNAME.ttf?wght=400" -type msdf -size 64 -imageout FONTNAME.png -json FONTNAME.json -potr -pxrange 6
	// Command 2: texconv FONTNAME.png -f R8G8B8A8_UNORM -m 1 -y
	// NOTE: msdf-atlas-gen can't output DDS, so convert the PNG afterwards to a dds via command 2.

	std::string stem = aPath.stem().string();

	std::filesystem::path atlasPath;
	std::filesystem::path fontPath;

	if (stem.ends_with("_M"))
	{
		stem.erase(stem.size() - 2);
		fontPath = aPath;
		atlasPath = aPath.parent_path() / (stem + "_A.dds");
	}
	else if (stem.ends_with("_A"))
	{
		stem.erase(stem.size() - 2);
		fontPath = aPath.parent_path() / (stem + "_M.json");
		atlasPath = aPath;
	}

	myFont = std::make_unique<Font>(stem);

	if (!GraphicsEngine::Get().CreateFontAsset(fontPath, atlasPath, *myFont))
	{
		SPDLOG_ERROR("Failed to create font [{}]!", aPath.string());

		return false;
	}

    return true;
}

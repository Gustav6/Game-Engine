#include "FontAsset.h"

#include <utility>
#include <memory>

#include "GraphicsEngine/Objects/Text/Font.h"

#include "GraphicsEngine/GraphicsEngine.hpp"

#include <tracy/core/Tracy.hpp>

#include "Logger/Logger.h"

DECLARE_LOG_CATEGORY_WITH_NAME(FontLog, FONT, Verbose);

DEFINE_LOG_CATEGORY(FontLog);

FontAsset::FontAsset() = default;
FontAsset::~FontAsset() = default;

const std::shared_ptr<Font>& FontAsset::GetFont() const
{
	return myFont;
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

	myFont = std::make_shared<Font>(stem);

	if (!GraphicsEngine::Get().CreateFontAsset(fontPath, atlasPath, *myFont))
	{
		LOG(FontLog, Error, "Failed to create font [{}]!", aPath.string());
		return false;
	}

	/*myDefaultFontMaterial = std::make_shared<Material>();

	MaterialDescription fontMaterialDesc;
	fontMaterialDesc.Name = "FontMaterial";
	fontMaterialDesc.Domain = MaterialDomain::Surface;
	fontMaterialDesc.ShadingModel = ShadingModel::Unlit;
	fontMaterialDesc.BlendMode = MaterialBlendMode::Alpha;
	fontMaterialDesc.PixelShader = myShaderRoot / "Material" / "FontMaterial_PS.hlsl";
	fontMaterialDesc.VertexShader = myShaderRoot / "Material" / "FontMaterial_VS.hlsl";

	if (!CreateMaterial(fontMaterialDesc, *myDefaultFontMaterial))
	{
		GELOG(Error, "Failed to create font material!");
		return false;
	}*/

    return true;
}

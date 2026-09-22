#include "FontAsset.h"

#include <utility>
#include <memory>

#include "GraphicsEngine/Objects/Font.h"
#include "GraphicsEngine/Objects/Texture.h"
#include "GraphicsEngine/GraphicsEngine.hpp"

#include <tracy/core/Tracy.hpp>

#include "Logger/Logger.h"

FontAsset::FontAsset()
{
    myFont = std::make_shared<Font>();
}

FontAsset::~FontAsset() = default;

const std::shared_ptr<Font>& FontAsset::GetFont() const
{
	return myFont;
}

bool FontAsset::Load(const std::filesystem::path& aPath)
{
	// Command - msdf-atlas-gen -varfont "FONTNAME.ttf?wght=400" -type msdf -size 64 -imageout FONTNAME.png -json FONTNAME.json -potr -pxrange 6
	// NOTE: msdf-atlas-gen can't output DDS, so convert the PNG afterwards.
	// Convert to DDS:      texconv FONTNAME.png -f R8G8B8A8_UNORM -m 1 -y

    aPath;

	//{
	//	std::filesystem::path defaultFontRoot = myContentRoot / "Fonts" / "CascadiaCode";

	//	if (!CreateFontAsset(defaultFontRoot / "cascadiacode.json", defaultFontRoot / "cascadiacode.dds", *myDefaultFont))
	//	{
	//		GELOG(Error, "Failed to create default font asset!");
	//		return false;
	//	}

	//	myDefaultFontMaterial = std::make_shared<Material>();

	//	MaterialDescription fontMaterialDesc;
	//	fontMaterialDesc.Name = "FontMaterial";
	//	fontMaterialDesc.Domain = MaterialDomain::Surface;
	//	fontMaterialDesc.ShadingModel = ShadingModel::Unlit;
	//	fontMaterialDesc.BlendMode = MaterialBlendMode::Alpha;
	//	fontMaterialDesc.PixelShader = myShaderRoot / "Material" / "FontMaterial_PS.hlsl";
	//	fontMaterialDesc.VertexShader = myShaderRoot / "Material" / "FontMaterial_VS.hlsl";

	//	if (!CreateMaterial(fontMaterialDesc, *myDefaultFontMaterial))
	//	{
	//		GELOG(Error, "Failed to create font material!");
	//		return false;
	//	}
	//}

    return false;
}

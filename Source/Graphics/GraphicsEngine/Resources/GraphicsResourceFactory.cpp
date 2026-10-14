#include "GraphicsEngine.pch.h"
#include "GraphicsResourceFactory.h"

#include <fstream>

#include "GraphicsEngine/RHI/RenderHardwareInterface.h"

#include "JsonWrapper.h"

#include "GraphicsEngine/Objects/Text/Font.h"

bool GraphicsResourceFactory::CreateShadowMap(RenderHardwareInterface& aRHI, std::string_view aName, unsigned int aWidth, unsigned int aHeight, Texture& outShadowMap, bool aCube)
{
	return aRHI.CreateDepthStencil(aName, aWidth, aHeight, outShadowMap, aCube);
}

bool GraphicsResourceFactory::CreateTexture(RenderHardwareInterface& aRHI, std::string_view aName, unsigned aWidth, unsigned aHeight, unsigned aNumMips, ResourceFormat aFormat, ResourceBindFlags aBindFlags, Texture& outTexture)
{
    return aRHI.CreateTexture(aName, aWidth, aHeight, aNumMips, aFormat, aBindFlags, outTexture);
}

bool GraphicsResourceFactory::CreateTexture(RenderHardwareInterface& aRHI, const std::filesystem::path& aPath, Texture& outTexture)
{
	if (!std::filesystem::exists(aPath))
	{
		spdlog::error("Failed to load Texture {}! File not found!", aPath.string());
		return false;
	}

	std::ifstream file(aPath, std::ios::binary | std::ios::ate);

	if (!file)
	{
		spdlog::error("Failed to load Texture {}! Could not open file!", aPath.string());
		return false;
	}

	const std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> fileData(size);
	file.read(reinterpret_cast<char*>(fileData.data()), size);
	file.close();

	return aRHI.CreateTexture(aPath.stem().string(), fileData.data(), fileData.size(), outTexture);
}

bool GraphicsResourceFactory::CreateFontAsset(RenderHardwareInterface& aRHI, const std::filesystem::path& aFontPath, const std::filesystem::path& aFontAtlasPath, Font& outFont)
{
	JsonData fontData = Json::Read(aFontPath);

	if (!fontData.IsValid())
	{
		spdlog::error("Failed to read font JSON [{}]!", aFontPath.string());
		return false;
	}

	JsonData atlasSettings = fontData.GetObjectValue("atlas");

	if (!atlasSettings.IsValid())
	{
		spdlog::error("Font JSON is missing [atlas]!");
		return false;
	}

	float width = static_cast<float>(atlasSettings.GetUnsigned("width"));
	float height = static_cast<float>(atlasSettings.GetUnsigned("height"));

	JsonData metricsSettings = fontData.GetObjectValue("metrics");

	if (!metricsSettings.IsValid())
	{
		spdlog::error("Font JSON is missing [metrics]!");
		return false;
	}

	outFont.myLineHeight = metricsSettings.GetFloat("lineHeight");
	outFont.myAscender = metricsSettings.GetFloat("ascender");
	outFont.myDescender = metricsSettings.GetFloat("descender");

	const std::vector<JsonData> glyphs = fontData.GetObjectValueArray("glyphs");

	if (glyphs.empty())
	{
		spdlog::error("Font JSON contains no glyphs [{}]!", aFontPath.string());
		return false;
	}

	for (const JsonData& glyphData : glyphs)
	{
		Font::Glyph glyph = {};

		glyph.UniCode = glyphData.GetUnsigned("unicode");
		glyph.Advance = glyphData.GetFloat("advance");

		if (glyphData.HasKey("planeBounds"))
		{
			JsonData planeBounds = glyphData.GetObjectValue("planeBounds");

			glyph.PlaneBounds.Left = planeBounds.GetFloat("left");
			glyph.PlaneBounds.Bottom = planeBounds.GetFloat("bottom");
			glyph.PlaneBounds.Right = planeBounds.GetFloat("right");
			glyph.PlaneBounds.Top = planeBounds.GetFloat("top");
		}

		if (glyphData.HasKey("atlasBounds"))
		{
			JsonData atlasBounds = glyphData.GetObjectValue("atlasBounds");

			glyph.AtlasBounds.Left = atlasBounds.GetFloat("left") / width;
			glyph.AtlasBounds.Bottom = atlasBounds.GetFloat("bottom") / height;
			glyph.AtlasBounds.Right = atlasBounds.GetFloat("right") / width;
			glyph.AtlasBounds.Top = atlasBounds.GetFloat("top") / height;
		}

		outFont.myGlyphs.emplace(static_cast<char>(glyph.UniCode), std::move(glyph));
	}

	return CreateTexture(aRHI, aFontAtlasPath, outFont.myAtlas);
}

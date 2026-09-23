#include "GraphicsEngine.pch.h"
#include "GraphicsResourceFactory.h"

#include "GraphicsEngine/RHI/RenderHardwareInterface.h"

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
		GELOG(Error, "Failed to load Texture {}! File not found!", aPath.string());
		return false;
	}

	std::ifstream file(aPath, std::ios::binary | std::ios::ate);

	if (!file)
	{
		GELOG(Error, "Failed to load Texture {}! Could not open file!", aPath.string());
		return false;
	}

	const std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> fileData(size);
	file.read(reinterpret_cast<char*>(fileData.data()), size);
	file.close();

	return aRHI.CreateTexture(aPath.stem().string(), fileData.data(), fileData.size(), outTexture);
}

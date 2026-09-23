#pragma once

#include <string_view>
#include <filesystem>

class RenderHardwareInterface;
class Texture;

enum class ResourceFormat;

class GraphicsResourceFactory
{
public:
    static bool CreateShadowMap(RenderHardwareInterface& aRHI, std::string_view aName, unsigned int aWidth, unsigned int aHeight, Texture& outShadowMap, bool aCube);

    static bool CreateTexture(RenderHardwareInterface& aRHI, std::string_view aName, unsigned aWidth, unsigned aHeight, unsigned aNumMips, ResourceFormat aFormat, ResourceBindFlags aBindFlags, Texture& outTexture);

    static bool CreateTexture(RenderHardwareInterface& aRHI, const std::filesystem::path& aPath, Texture& outTexture);
};
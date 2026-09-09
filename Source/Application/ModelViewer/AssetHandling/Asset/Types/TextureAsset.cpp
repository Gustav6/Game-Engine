#include "TextureAsset.h"

#include "GraphicsEngine/GraphicsEngine.hpp"

#include <tracy/core/Tracy.hpp>

TextureAsset::~TextureAsset() = default;

const Texture& TextureAsset::GetTexture() const
{
    return myTexture;
}

bool TextureAsset::Load(const std::filesystem::path& aPath)
{
    ZoneScopedN("Load Texture");

    if (!GraphicsEngine::Get().LoadTexture(aPath, myTexture))
    {
        return false;
    }

    LOG(TextureLog, Verbose, "Texture has loaded [{}]", aPath.string());

    return true;
}

#include "GameFramework.pch.h"
#include "TextureAsset.h"

#include "GraphicsEngine/GraphicsEngine.hpp"
#include "GraphicsEngine/Objects/Texture.h"

#include <tracy/core/Tracy.hpp>

#include "Logger/Logger.h"

DECLARE_LOG_CATEGORY_WITH_NAME(TextureLog, TEXTURE, Verbose);

DEFINE_LOG_CATEGORY(TextureLog);

TextureAsset::~TextureAsset() = default;

const std::shared_ptr<Texture>& TextureAsset::GetTexture() const
{
    return myTexture;
}

bool TextureAsset::Load(const std::filesystem::path& aPath)
{
    ZoneScopedN("Load Texture");

    myTexture = std::make_shared<Texture>();

    if (!GraphicsEngine::Get().CreateTexture(aPath, *myTexture))
    {
        return false;
    }

    return true;
}

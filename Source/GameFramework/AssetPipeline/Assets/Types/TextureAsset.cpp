#include "GameFramework.pch.h"
#include "TextureAsset.h"

#include "GraphicsEngine/GraphicsEngine.hpp"

#include "GraphicsEngine/Objects/GPU/Texture.h"

#include <tracy/core/Tracy.hpp>

#include "Logger/Logger.h"

DECLARE_LOG_CATEGORY_WITH_NAME(LogTexture, TEXTURE, Verbose);

DEFINE_LOG_CATEGORY(LogTexture);

#include "Logger/Logger.h"

TextureAsset::~TextureAsset() = default;

const std::shared_ptr<Texture>& TextureAsset::GetTexture() const
{
    return myTexture;
}

bool TextureAsset::Load(const std::filesystem::path& aPath)
{
    myTexture = std::make_shared<Texture>();

    if (!GraphicsEngine::Get().CreateTexture(aPath, *myTexture))
    {
        LOG(LogTexture, Error, "Could not create a texture for {}!", aPath.string());
        return false;
    }

    return true;
}

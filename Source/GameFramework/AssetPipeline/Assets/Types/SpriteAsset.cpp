#include "GameFramework.pch.h"
#include "SpriteAsset.h"

#include <GraphicsEngine/GraphicsEngine.hpp>

#include "GraphicsEngine/Objects/GPU/Texture.h"
#include "GraphicsEngine/Objects/Sprite.h"

#include <tracy/core/Tracy.hpp>

#include "Logger/Logger.h"

DECLARE_LOG_CATEGORY_WITH_NAME(SpriteLog, SPRITE, Verbose);

DEFINE_LOG_CATEGORY(SpriteLog);

SpriteAsset::SpriteAsset()
{
	myData = std::make_unique<SpriteSharedData>();
}

SpriteAsset::~SpriteAsset() = default;

const SpriteSharedData& SpriteAsset::GetData() const
{
	return *myData;
}

bool SpriteAsset::Load(const std::filesystem::path& aPath)
{
	if (!GraphicsEngine::Get().CreateTexture(aPath, myData->Texture))
	{
		LOG(SpriteLog, Error, "Could not create a sprite texture for {}!", aPath.string());
		return false;
	}

	myData->Name = aPath.stem().string();

	return true;
}

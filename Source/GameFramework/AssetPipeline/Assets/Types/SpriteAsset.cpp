#include "GameFramework.pch.h"
#include "SpriteAsset.h"

#include <GraphicsEngine/GraphicsEngine.hpp>

#include "GraphicsEngine/Objects/GPU/Texture.h"
#include "GraphicsEngine/Objects/Sprite.h"

#include <tracy/core/Tracy.hpp>

#include "Logger/Logger.h"

const SpriteSharedData& SpriteAsset::GetData() const
{
	return *myData;
}

bool SpriteAsset::Load(const std::filesystem::path& aPath)
{
	myData = std::make_unique<SpriteSharedData>();

	if (!GraphicsEngine::Get().CreateTexture(aPath, myData->Texture))
	{
		spdlog::error("Could not create a sprite texture for {}!", aPath.string());
		return false;
	}

	myData->Name = aPath.stem().string();

	return true;
}

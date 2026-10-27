#include "Core.pch.h"
#include "SpriteAsset.h"

#include <Graphics/GraphicsEngine.hpp>

#include "Graphics/Objects/GPU/Texture.h"
#include "Graphics/Objects/Sprite.h"


const SpriteSharedData& SpriteAsset::GetData() const
{
	return *myData;
}

bool SpriteAsset::Load(const std::filesystem::path& aPath)
{
	myData = std::make_unique<SpriteSharedData>();

	if (!GraphicsEngine::Get().CreateTexture(aPath, myData->Texture))
	{
		SPDLOG_ERROR("Could not create a sprite texture for {}!", aPath.string());
		return false;
	}

	myData->Name = aPath.stem().string();

	return true;
}

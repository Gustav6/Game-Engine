#include "SpriteAsset.h"

#include "GraphicsEngine/Objects/GPU/Texture.h"
#include "GraphicsEngine/Objects/Sprite.h"

#include <tracy/core/Tracy.hpp>

#include "Logger/Logger.h"
#include "tgafbx/Importer.h"
#include "tgafbx/TgaFbxStructs.h"
#include <GraphicsEngine/GraphicsEngine.hpp>
#include "../../../../Application/Game/Application.h"


DECLARE_LOG_CATEGORY_WITH_NAME(SpriteLog, SPRITE, Verbose);

DEFINE_LOG_CATEGORY(SpriteLog);

SpriteAsset::SpriteAsset()
{
	mySpriteShared = std::make_unique<SpriteShared>();
}

SpriteAsset::~SpriteAsset() = default;

const SpriteShared& SpriteAsset::GetSprite() const
{
	return *mySpriteShared;
}

bool SpriteAsset::Load(const std::filesystem::path& aPath)
{
	GraphicsEngine& GE = GraphicsEngine::Get();
	
	mySpriteShared->TextureResource = std::make_shared<Texture>();

	if (!GE.CreateTexture(aPath, *mySpriteShared->TextureResource))
	{
		return false;
	}


	return true;
}

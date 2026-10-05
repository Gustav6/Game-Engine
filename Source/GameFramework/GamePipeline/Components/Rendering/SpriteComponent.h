#pragma once

#include "GamePipeline/Components/Base/Component.h"

#include <memory>

#include <AssetPipeline/Assets/Types/SpriteAsset.h>

#include "GraphicsEngine/Objects/Sprite.h"

#include <CommonUtilities/Math/Vector2.hpp>
#include <CommonUtilities/Math/Vector4.hpp>

class Texture;

class SpriteComponent : public Component
{
public:
	SpriteComponent(Actor* aOwner);

	void Render() override;

	void SetSprite(const std::shared_ptr<SpriteAsset>& aSpriteAsset);

	const SpriteInstanceData& GetInstanceData() const;
	const SpriteSharedData& GetSharedData() const;

	void SetSize(const CommonUtilities::Vector2f& aSize);
	void SetPivot(const CommonUtilities::Vector2f& aPivot);
	void SetAnchor(const CommonUtilities::Vector2f& aAnchor);
	void SetPosition(const CommonUtilities::Vector2f& aPosition);
	void SetColor(const CommonUtilities::Vector4f& aColor);
	void SetDepth(float aDepth);
	void SetVisible(bool aVisible);

private:
	std::shared_ptr<SpriteAsset> mySpriteAsset;
	SpriteInstanceData mySpriteInstance;
};


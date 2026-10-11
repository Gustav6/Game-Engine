#pragma once

#include "GamePipeline/Components/Base/Component.h"

#include <memory>

#include "GraphicsEngine/Objects/Sprite.h"

#include <CommonUtilities/Math/Vector2.hpp>
#include <CommonUtilities/Math/Vector4.hpp>

class Texture;
class MaterialAsset;
class SpriteAsset;

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
	void SetMaterial(const std::shared_ptr<MaterialAsset>& aMaterial);

private:
	std::shared_ptr<SpriteAsset> mySpriteAsset;
	std::shared_ptr<MaterialAsset> myMaterialAsset;
	SpriteInstanceData mySpriteInstance;
};


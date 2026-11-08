#include "Core/Core.pch.h"
#include "SpriteComponent.h"

#include "Graphics/Drawers/SpriteDrawer.h"

#include "Core/Assets/Concrete/MaterialAsset.h"
#include "Core/Assets/Concrete/SpriteAsset.h"


SpriteComponent::SpriteComponent(Actor* aOwner)
	: Component(aOwner)
{
}

void SpriteComponent::Render()
{
	if (mySpriteAsset == nullptr)
	{
		return;
	}

	MaterialInterface* material = nullptr;

	if (myMaterialAsset != nullptr)
	{
		material = myMaterialAsset->GetMaterial().get();
	}

	SpriteDrawCall drawCall =
	{
		.SpriteInstanceData = &mySpriteInstance,
		.SpriteSharedData = &mySpriteAsset->GetData(),
		.Material = material
	};

	SpriteDrawer::Get().Submit(drawCall);
}

void SpriteComponent::SetSprite(const std::shared_ptr<SpriteAsset>& aSpriteAsset)
{
	mySpriteAsset = aSpriteAsset;
}

const SpriteInstanceData& SpriteComponent::GetInstanceData() const
{
	return mySpriteInstance;
}

const SpriteSharedData& SpriteComponent::GetSharedData() const
{
	return mySpriteAsset->GetData();
}

void SpriteComponent::SetSize(const CommonUtilities::Vector2f& aSize)
{
	mySpriteInstance.Size = aSize;
}

void SpriteComponent::SetPivot(const CommonUtilities::Vector2f& aPivot)
{
	mySpriteInstance.Pivot = aPivot;
}

void SpriteComponent::SetAnchor(const CommonUtilities::Vector2f& aAnchor)
{
	mySpriteInstance.Anchor = aAnchor;
}

void SpriteComponent::SetPosition(const CommonUtilities::Vector2f& aPosition)
{
	mySpriteInstance.Position.x = aPosition.x;
	mySpriteInstance.Position.y = aPosition.y;
}

void SpriteComponent::SetColor(const CommonUtilities::Vector4f& aColor)
{
	mySpriteInstance.Color = aColor;
}

void SpriteComponent::SetDepth(float aDepth)
{
	mySpriteInstance.Depth = aDepth;
}

void SpriteComponent::SetVisible(bool aVisible)
{
	mySpriteInstance.IsVisible = aVisible;
}

void SpriteComponent::SetMaterial(const std::shared_ptr<MaterialAsset>& aMaterial)
{
	myMaterialAsset = aMaterial;
}
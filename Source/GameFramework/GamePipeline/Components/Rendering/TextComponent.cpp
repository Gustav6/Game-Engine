#include "GameFramework.pch.h"
#include "TextComponent.h"

#include <CommonUtilities/Math/Vector3.hpp>

#include <AssetPipeline/Assets/Types/FontAsset.h>
#include <AssetPipeline/Assets/Types/MaterialAsset.h>

#include <GraphicsEngine/Drawers/TextDrawer.h>

TextComponent::TextComponent(Actor* aOwner)
	: Component(aOwner)
{
	myIsDirty = true;
	mySize = 12.0f;
	myText = std::make_unique<Text>();
}

void TextComponent::Render()
{
	if (myIsDirty)
	{
		RebuildTransform();
		myIsDirty = false;
	}

	TextDrawCall drawCall =
	{
		.Text = myText.get(),
		.Font = &myFontAsset->GetFont(),
		.Material = nullptr,
		.Transform = myTransform
	};

	TextDrawer::Get().Submit(drawCall);
}

const CommonUtilities::Matrix4x4f& TextComponent::GetTransform() const
{
	return myTransform;
}

void TextComponent::SetFont(const std::shared_ptr<FontAsset>& aFontAsset)
{
	myFontAsset = aFontAsset;
}

void TextComponent::SetText(std::string_view aText)
{
	myText->SetText(aText);
	myIsDirty = true;
}

void TextComponent::SetSize(float aPointSize)
{
	mySize = aPointSize;
	myText->SetSize(aPointSize);
	myIsDirty = true;
}

void TextComponent::SetDepth(float aDepth)
{
	myText->SetDepth(aDepth);
	myIsDirty = true;
}

void TextComponent::SetColor(const CommonUtilities::Vector4f& aColor)
{
	myText->SetColor(aColor);
	myIsDirty = true;
}

void TextComponent::SetPosition(const CommonUtilities::Vector2f& aPosition)
{
	myPosition = aPosition;
	myIsDirty = true;
}

void TextComponent::RebuildTransform()
{
	const float pointSize = mySize;
	const float dpi = 96.0f;
	const float fontPixelHeight = pointSize * (dpi / 72.0f);

	CommonUtilities::Matrix4x4f scaling;
	scaling.SetScaling({ fontPixelHeight, -fontPixelHeight, 1.0f });

	CommonUtilities::Matrix4x4f translation;
	translation.SetTranslation({ myPosition.x, myPosition.y, 0});

	myTransform = scaling * translation;
}

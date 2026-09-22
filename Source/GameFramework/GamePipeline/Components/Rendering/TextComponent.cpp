#include "TextComponent.h"

#include <AssetPipeline/Assets/Types/FontAsset.h>
#include <AssetPipeline/Assets/Types/MaterialAsset.h>

TextComponent::TextComponent(Actor* aOwner)
	: Component(aOwner)
{
	myText = std::make_unique<Text>("Text");
}

void TextComponent::SetFont(const std::shared_ptr<FontAsset>& aFontAsset)
{
	myText->SetFont(aFontAsset->GetFont());
}

const Text& TextComponent::GetText()
{
	return *myText;
}

void TextComponent::SetText(std::string_view aText)
{
	myText->SetText(aText);
}

void TextComponent::SetSize(float aPointSize)
{
	myText->SetSize(aPointSize);
}

void TextComponent::SetZOrder(float aOrder)
{
	myText->SetZOrder(aOrder);
}

void TextComponent::SetColor(const CommonUtilities::Vector4f& aColor)
{
	myText->SetColor(aColor);
}

void TextComponent::SetPosition(const CommonUtilities::Vector2f& aPosition)
{
	myText->SetPosition(aPosition);
}

#pragma once

#include "../Base/Component.h"

#include <memory>
#include <string_view>


#include "GraphicsEngine/Objects/Text.h"

#include <CommonUtilities/Math/Vector2.hpp>
#include <CommonUtilities/Math/Vector4.hpp>

class FontAsset;
class MaterialAsset;

class TextComponent : public Component
{
public:
	TextComponent(Actor* aOwner);

	void SetFont(const std::shared_ptr<FontAsset>& aFontAsset);

	const Text& GetText();

	void SetText(std::string_view aText);
	void SetSize(float aPointSize);
	void SetZOrder(float aOrder);
	void SetColor(const CommonUtilities::Vector4f& aColor);
	void SetPosition(const CommonUtilities::Vector2f& aPosition);

private:
	std::unique_ptr<Text> myText;
};


#pragma once

#include "GamePipeline/Components/Base/Component.h"

#include <memory>
#include <string_view>

#include "GraphicsEngine/Objects/Text/Text.h"

#include <CommonUtilities/Math/Vector2.hpp>
#include <CommonUtilities/Math/Vector4.hpp>
#include <CommonUtilities/Math/Matrix4x4.hpp>

class FontAsset;
class MaterialAsset;

class TextComponent : public Component
{
public:
	TextComponent(Actor* aOwner);

	void Render() override;

	Text& GetText();
	Font& GetFont();

	const CommonUtilities::Matrix4x4f& GetTransform() const;

	void SetFont(const std::shared_ptr<FontAsset>& aFontAsset);
	void SetText(std::string_view aText);
	void SetSize(float aPointSize);
	void SetDepth(float aDepth);
	void SetColor(const CommonUtilities::Vector4f& aColor);
	void SetPosition(const CommonUtilities::Vector2f& aPosition);

private:
	void RebuildTransform();

	std::unique_ptr<Text> myText;
	std::shared_ptr<FontAsset> myFontAsset;

	CommonUtilities::Vector2f myPosition;
	CommonUtilities::Matrix4x4f myTransform;

	float mySize;

	bool myIsDirty;
};


#pragma once

#include <vector>

#include <GraphicsEngine/Materials/Material.h>

#include "CommonUtilities/Math/Matrix4x4.hpp"

class Text;
class Font;
class MaterialInterface;

struct TextDrawCall
{
    Text* Text = nullptr;
	const Font* Font = nullptr;
	const MaterialInterface* Material;
	CommonUtilities::Matrix4x4f Transform;
};

class TextDrawer
{
public:
	static TextDrawer& Get();

	void Submit(const TextDrawCall& aDrawCall);

	const std::vector<TextDrawCall>& GetTextDrawCalls() const;

	void Clear();

private:
	std::vector<TextDrawCall> myDrawCalls;
};
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
	friend class GraphicsEngine;
	friend class ScreenSpaceRenderPass;

public:
	static TextDrawer& Get();

	void Submit(const TextDrawCall& aDrawCall);

private:
	std::vector<TextDrawCall> myDrawCalls;
};
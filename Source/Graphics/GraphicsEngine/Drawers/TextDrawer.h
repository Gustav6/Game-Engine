#pragma once

#include <vector>

#include <GraphicsEngine/Materials/Material.h>

class Text;

struct TextDrawCall
{
    Text* Text = nullptr;
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
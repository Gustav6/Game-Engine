#pragma once

#include <unordered_map>

#include "CommonUtilities/Rectangle.hpp"
#include "GraphicsEngine/Objects/GPU/Texture.h"

class Font 
{
	friend class GraphicsEngine;
	friend class ScreenSpaceRenderPass;

public:
	Font(std::string_view aName);

	struct Glyph 
	{
		unsigned int UniCode;
		float Advance;

		CommonUtilities::Rectanglef PlaneBounds;
		CommonUtilities::Rectanglef AtlasBounds;
	};

	const Glyph* GetGlyph(char aGlyph) const;
	float GetLineHeight() const;
	float GetAscender() const;
	float GetDescender() const;

private:
	// The height of a line of text, in font units
	float myLineHeight;

	// The maximum height above the line a glyph can reach.
	float myAscender;

	// The maximum height below the line a glyph can reach.
	float myDescender;

	std::string myName;

	std::unordered_map<char, Glyph> myGlyphs;
	Texture myAtlas;
};
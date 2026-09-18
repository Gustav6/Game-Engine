#pragma once

#include <unordered_map>

#include "CommonUtilities/Rectangle.hpp"
#include "Texture.h"

class Font 
{
	friend class GraphicsEngine;

public:
	struct Glyph 
	{
		unsigned int UniCode;
		float Advance;

		CommonUtilities::Rectanglef PlaneBounds;
		CommonUtilities::Rectanglef AtlasBounds;
	};

private:
	// The height of a line of text, in font units
	float myLineHeight;

	// The maximum height above the line a glyph can reach.
	float myAscender;

	// The maximum height below the line a glyph can reach.
	float myDescender;

	std::unordered_map<char, Glyph> myGlyphs;
	Texture myAtlas;

public:
	const Glyph* GetGlyph(char aGlyph) const;
	float GetLineHeight() const { return myLineHeight; };
	float GetAscender() const { return myAscender; };
	float GetDescender() const { return myDescender; };
};
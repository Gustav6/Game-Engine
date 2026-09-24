#include "Font.h"

Font::Font(std::string_view aName)
{
	myName = aName;
	myLineHeight = 0;
	myAscender = 0;
	myDescender = 0;
}

const Font::Glyph* Font::GetGlyph(char aGlyph) const
{
	auto iterator = myGlyphs.find(aGlyph);

	if (iterator == myGlyphs.end())
	{
		return nullptr;
	}

	return &iterator->second;
}

float Font::GetLineHeight() const
{
	return myLineHeight;
}

float Font::GetAscender() const
{
	return myAscender;
}

float Font::GetDescender() const
{
	return myDescender;
}

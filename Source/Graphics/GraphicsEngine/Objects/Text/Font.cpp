#include "Font.h"

const Font::Glyph* Font::GetGlyph(char aGlyph) const
{
	auto iterator = myGlyphs.find(aGlyph);

	if (iterator == myGlyphs.end())
	{
		return nullptr;
	}

	return &iterator->second;
}

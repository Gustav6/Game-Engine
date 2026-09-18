#include "Font.h"

const Font::Glyph* Font::GetGlyph(char aGlyph) const
{
	return &myGlyphs.at(aGlyph);
}

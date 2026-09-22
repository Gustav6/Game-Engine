#include "TextDrawer.h"

TextDrawer& TextDrawer::Get()
{
	static TextDrawer myInstance;
	return myInstance;
}

void TextDrawer::Submit(const TextDrawCall& aDrawCall)
{
	myDrawCalls.emplace_back(aDrawCall);
}

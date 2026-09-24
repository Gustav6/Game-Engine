#include "GraphicsEngine.pch.h"
#include "TextDrawer.h"

#include "GraphicsEngine/Objects/Text/Text.h"

TextDrawer& TextDrawer::Get()
{
	static TextDrawer myInstance;
	return myInstance;
}

void TextDrawer::Submit(const TextDrawCall& aDrawCall)
{
	if (aDrawCall.Text == nullptr || aDrawCall.Text->myFont == nullptr)
	{
		GELOG(Error, "Text was submitted with invalid data!");
		return;
	}

	myDrawCalls.emplace_back(aDrawCall);
}

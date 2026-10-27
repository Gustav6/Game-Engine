#include "GraphicsEngine.pch.h"
#include "TextDrawer.h"

#include "Graphics/Objects/Text/Text.h"

TextDrawer& TextDrawer::Get()
{
	static TextDrawer myInstance;
	return myInstance;
}

void TextDrawer::Submit(const TextDrawCall& aDrawCall)
{
	if (aDrawCall.Text == nullptr || aDrawCall.Font == nullptr)
	{
		SPDLOG_ERROR("Text was submitted with invalid data!");
		return;
	}

	myDrawCalls.emplace_back(aDrawCall);
}

const std::vector<TextDrawCall>& TextDrawer::GetTextDrawCalls() const
{
	return myDrawCalls;
}

void TextDrawer::Clear()
{
	myDrawCalls.clear();
}

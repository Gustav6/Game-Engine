#include "GraphicsEngine.pch.h"
#include "SpriteDrawer.h"

#include "GraphicsEngine/Objects/Sprite.h"

SpriteDrawer& SpriteDrawer::Get()
{
	static SpriteDrawer myInstance;
	return myInstance;
}

void SpriteDrawer::Submit(const SpriteDrawCall& aDrawCall)
{
	if (aDrawCall.SpriteSharedData == nullptr || aDrawCall.SpriteInstanceData == nullptr)
	{
		SPDLOG_ERROR("Sprite was submitted with invalid data!");
		return;
	}

	if (!aDrawCall.SpriteInstanceData->IsVisible)
	{
		return;
	}

	myDrawCalls.emplace_back(aDrawCall);
}

const std::vector<SpriteDrawCall>& SpriteDrawer::GetSpriteDrawCalls() const
{
	return myDrawCalls;
}

void SpriteDrawer::Clear()
{
	myDrawCalls.clear();
}

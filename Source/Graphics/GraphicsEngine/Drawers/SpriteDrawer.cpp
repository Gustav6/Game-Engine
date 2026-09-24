#include "SpriteDrawer.h"

#include "GraphicsEngine/Objects/Sprite.h"

SpriteDrawer& SpriteDrawer::Get()
{
	static SpriteDrawer myInstance;
	return myInstance;
}

void SpriteDrawer::Submit(const SpriteDrawCall& aDrawCall)
{
	if (!aDrawCall.SpriteInstance->IsVisible)
	{
		return;
	}

	myDrawCalls.emplace_back(aDrawCall);
}

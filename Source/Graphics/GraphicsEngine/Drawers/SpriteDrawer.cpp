#include "SpriteDrawer.h"

SpriteDrawer& SpriteDrawer::Get()
{
	static SpriteDrawer myInstance;
	return myInstance;
}

#pragma once

#include <vector>

#include "Materials/Material.h"

struct SpriteDrawCall
{};

class SpriteDrawer
{
public:
	static SpriteDrawer& Get();
};
#pragma once

#include <array>

#include "Texture.h"

static constexpr unsigned NUM_GBUFFER_TARGETS = 5;

class GBuffer
{
public:
	const Texture* GetTextures() const;
	
private:
	std::array<Texture, NUM_GBUFFER_TARGETS> myTextures;
};


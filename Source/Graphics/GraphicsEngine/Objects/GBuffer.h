#pragma once

#include <array>

#include "Texture.h"

class GBuffer
{
	friend class RenderHardwareInterface;
	friend class GraphicsCommandList;

public:
	GBuffer();
	~GBuffer();

	const Texture* GetTextures() const;
	static constexpr int NUM_GBUFFER_TARGETS = 5;

private:
	std::array<Texture, NUM_GBUFFER_TARGETS> myTextures;
	std::string myName;
};


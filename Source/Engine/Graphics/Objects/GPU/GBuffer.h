#pragma once

#include <array>

#include "Texture.h"

class GBuffer
{
	friend class RenderHardwareInterface;
	friend class GraphicsCommandList;

public:
	enum class Target : uint8_t
	{
		Albedo,
		NormalDepth,
		Material,
		Emission,
		WorldPosition,
		MAX
	};

	GBuffer();
	~GBuffer();

	const Texture* GetTextures() const;
	static constexpr int NUM_GBUFFER_TARGETS = static_cast<int>(Target::MAX);

private:
	std::array<Texture, NUM_GBUFFER_TARGETS> myTextures;
	std::string myName;
};


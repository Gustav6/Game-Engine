#pragma once

#include <cstdint>

enum class ConstantBuffer : uint8_t
{
	FrameBuffer,
	ObjectBuffer,
	AnimationBuffer,
	LightBuffer,
	MaterialBuffer,
	CascadeBuffer,
	MAX
};
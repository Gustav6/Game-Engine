#pragma once

#include "CommonUtilities/Matrix4x4.hpp"

struct Animation
{
	struct Frame
	{
		std::unordered_map<std::string, CommonUtilities::Matrix4x4f> Transforms;
	};

	std::vector<Frame> Frames;
	float Duration = 0;
	float FramesPerSecond = 0;
};
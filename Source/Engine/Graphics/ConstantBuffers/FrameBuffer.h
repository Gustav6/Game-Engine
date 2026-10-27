#pragma once

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector3.hpp"

struct FrameBuffer
{
	CommonUtilities::Matrix4x4f View;
	CommonUtilities::Matrix4x4f Projection;
	CommonUtilities::Vector3f CameraPosition;
	unsigned int TypeOfRenderPass = 0;
	unsigned int TypeOfTonemap = 0;
	unsigned int SSAOFlag = 0;
	CommonUtilities::Vector2f _padding;
};
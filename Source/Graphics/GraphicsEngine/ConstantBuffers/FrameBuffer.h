#pragma once

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector3.hpp"

struct FrameBuffer
{
	CommonUtilities::Matrix4x4f View;
	CommonUtilities::Matrix4x4f Projection;
	CommonUtilities::Vector3f CameraPosition;
	unsigned int TypeOfRenderPass = 0;
};
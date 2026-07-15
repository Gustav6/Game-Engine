#pragma once

#include "CommonUtilities/Matrix4x4.hpp"
#include "CommonUtilities/Vector3.hpp"

struct ObjectBuffer
{
	CommonUtilities::Matrix4x4f World;
	CommonUtilities::Matrix4x4f WorldInvT;
	unsigned int HasSkinning = false;
	CommonUtilities::Vector3f _padding;
};
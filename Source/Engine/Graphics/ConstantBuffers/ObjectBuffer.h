#pragma once

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector2.hpp"

struct ObjectBuffer
{
	CommonUtilities::Matrix4x4f World;
	CommonUtilities::Matrix4x4f WorldInvT;
	unsigned int HasSkinning = false;
	unsigned int ObjectID = 0;
	CommonUtilities::Vector2f _padding;
};
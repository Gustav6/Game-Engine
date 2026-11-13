#pragma once

#include "CommonUtilities/Math/Matrix4x4.hpp"

struct ObjectBuffer
{
	CommonUtilities::Matrix4x4f World;
	CommonUtilities::Matrix4x4f WorldInvT;
	unsigned int HasSkinning = false;
	unsigned int IsInstanced = false;
	unsigned int ObjectID = 0;
	float _padding;
};
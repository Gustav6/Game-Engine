#pragma once

#include "CommonUtilities/Matrix4x4.hpp"

struct AnimationBuffer
{
	CommonUtilities::Matrix4x4f JointTransforms[128];
};
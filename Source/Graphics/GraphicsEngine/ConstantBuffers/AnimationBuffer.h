#pragma once

#include "CommonUtilities/Math/Matrix4x4.hpp"

struct AnimationBuffer
{
	CommonUtilities::Matrix4x4f JointTransforms[128];
};
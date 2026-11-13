#pragma once

#include <CommonUtilities/Math/Matrix4x4.hpp>

struct InstanceData
{
	CommonUtilities::Matrix4x4f World;
	CommonUtilities::Matrix4x4f WorldInvT;
};
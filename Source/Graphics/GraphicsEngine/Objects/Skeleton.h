#pragma once

#include <vector>
#include <unordered_map>

#include "CommonUtilities/Matrix4x4.hpp"

struct Skeleton
{
	struct Joint
	{
		CommonUtilities::Matrix4x4f BindPoseInverse;
		int Parent = -1;
		std::vector<unsigned int> Children;
		std::string Name;
	};

	std::vector<Joint> Joints;
	std::unordered_map<std::string, size_t> JointNameToIndex;
};
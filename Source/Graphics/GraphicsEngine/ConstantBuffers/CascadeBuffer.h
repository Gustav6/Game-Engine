#pragma once

#include "CommonUtilities/Matrix4x4.hpp"
#include "CommonUtilities/Vector3.hpp"

// FYI: Number of cascades is set to 3

struct CascadeBuffer
{
	CommonUtilities::Matrix4x4f LightViewProjectionTextures[3];
	CommonUtilities::Vector3f CascadeFarPlanes;
	float _padding1;
	CommonUtilities::Vector3f CameraPosition;
	float _padding2;
};


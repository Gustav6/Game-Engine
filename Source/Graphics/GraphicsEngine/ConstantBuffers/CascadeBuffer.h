#pragma once

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector3.hpp"

struct CascadeBuffer
{
	CommonUtilities::Matrix4x4f LightViewProjectionTextures[4];
	CommonUtilities::Vector4f FarPlanes = { 0, 0, 0, 0 };
	// The sizes need to be uniform so use either. (width = height)
	CommonUtilities::Vector4f ShadowMapSizes  = { 0, 0, 0, 0 };
	CommonUtilities::Vector3f CameraPosition = { 0, 0, 0 };
    float _padding;
};


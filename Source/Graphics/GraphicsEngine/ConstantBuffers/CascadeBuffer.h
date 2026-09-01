#pragma once

#include "CommonUtilities/Matrix4x4.hpp"
#include "CommonUtilities/Vector3.hpp"

struct CascadeBuffer
{
	CommonUtilities::Matrix4x4f LightViewProjectionTextures[4];
	CommonUtilities::Vector4f FarPlanes = { 0, 0, 0, 0 };
	CommonUtilities::Vector4f ShadowMapSizes  = { 0, 0, 0, 0 };
	CommonUtilities::Vector3f CameraPosition = { 0, 0, 0 };
    float _padding;
};


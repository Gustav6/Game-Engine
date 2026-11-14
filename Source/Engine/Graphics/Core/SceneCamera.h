#pragma once

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "Graphics/Objects/Scene/Camera.h"

struct CameraData
{
	CommonUtilities::Matrix4x4f Transform;
	Camera RenderCamera;
};
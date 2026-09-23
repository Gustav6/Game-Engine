#pragma once

#include "CommonUtilities/Math/Matrix4x4.hpp"

class Camera;

struct SceneCamera
{
	CommonUtilities::Matrix4x4f Transform;
	const Camera* RenderCamera = nullptr;
};
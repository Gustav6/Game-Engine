#pragma once

#include "CommonUtilities/Matrix4x4.hpp"
#include "CommonUtilities/Frustum.hpp"

#include "GraphicsEngine/Objects/Camera.h"

#include "GameObject.hpp"

class GameCamera : public GameObject
{
public:
	GameCamera();
	GameCamera(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution);

	void UpdateFrustum();

	const Camera& GetCamera() const;

private:
	Camera myCamera;
};
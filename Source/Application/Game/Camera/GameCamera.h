#pragma once

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector2.hpp"
#include "CommonUtilities/Collision/Frustum.hpp"

#include "GamePipeline/World/Actor.hpp"

class Camera;

class GameCamera : public Actor
{
public:
	GameCamera();
	GameCamera(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution);

	// TODO : Make this run on transform dirty
	void UpdateFrustum();

	const Camera& GetCamera() const;

private:
	std::unique_ptr<Camera> myCamera;
};
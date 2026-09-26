#pragma once

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector2.hpp"
#include "CommonUtilities/Collision/Frustum.hpp"

#include "GamePipeline/World/Actor.hpp"

class Camera;

enum class Mode
{
	FreeLook,
	LookAt,
	LookAtLocked,
};

class GameCamera : public Actor
{
public:
	static GameCamera& GetInstance();

	void Init(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution);

	// TODO : Make this run on transform dirty
	void Update(float aDeltaTime, Transform& aPlyerTransform);

	void UpdateFrustum();

	void UpdateMovment(float aDeltaTime);
	void UpdateLockAt(Transform& aPlyerTransform, float aDeltaTime);
	void UpdateLockAtLocked(Transform& aPlyerTransform, float aDeltaTime);
	void UpdateFreeLock(float aDeltaTime);
	Mode GetMode() const;

	const Camera& GetRenderCamera() const;

private:
	GameCamera() = default;
	GameCamera(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution);
	std::unique_ptr<Camera> myCamera;

	Mode myMode;

	float myCamerasMoveSpeed;
};
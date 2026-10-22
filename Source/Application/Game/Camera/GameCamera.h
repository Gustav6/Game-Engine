#pragma once

#include "CommonUtilities/Math/Vector2.hpp"
#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Collision/Ray.hpp"

#include "GamePipeline/World/Actor.hpp"

class Camera;

enum class CameraMode
{
	FollowTarget,
	FreeLook
};

class GameCamera : public Actor
{
public:
	static GameCamera& GetInstance();

	void Init(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution);

	void Update(float aDeltaTime) override;

	void SetTarget(const Actor* aTarget);
	void SetMode(CameraMode aMode);

	const Camera& GetRenderCamera() const;

	CommonUtilities::Ray<float> ScreenToWorldRay(const CommonUtilities::Vector2f& aScreenPosition) const;
	bool WorldToScreen(const CommonUtilities::Vector3f& aWorldPosition, CommonUtilities::Vector2f& outScreenPosition) const;

private:
	GameCamera() = default;

	void UpdateFrustum() const;
	void UpdateFreeLook(float aDeltaTime);
	void UpdateFollowTarget(float aDeltaTime);

	const Actor* myTarget;

	std::unique_ptr<Camera> myRenderCamera;

	CameraMode myMode = CameraMode::FreeLook;

	CommonUtilities::Vector3f myShakeOffset = { 0.f, 0.f, 0.f };

	float myShakeIntensity = 0.f;
	float myShakeDuration = 0.f;
	float myShakeTimer = 0.f;

	float myCameraMoveSpeed = 400.0f;
	float myMouseSensitivity = 0.2f;
	float myFollowDistance = 300.0f;
	float myFollowHeight = 100.0f;
};
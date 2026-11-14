#pragma once

#include "CommonUtilities/Math/Vector2.hpp"
#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Collision/Ray.hpp"

#include "Core/Scene/World/Actor.hpp"

struct CameraData;

enum class CameraMode
{
	FollowTarget,
	FreeLook,
	LookAtTarget,
};

class GameCamera : public Actor
{
public:
	GameCamera(unsigned int aID);

	void Init(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution);

	void Update(float aDeltaTime) override;

	void SetTarget(const Actor* aTarget);
	void SetMode(CameraMode aMode);

	const CameraData& GetCameraData() const;

	CommonUtilities::Ray<float> ScreenToWorldRay(const CommonUtilities::Vector2f& aScreenPosition) const;
	bool WorldToScreen(const CommonUtilities::Vector3f& aWorldPosition, CommonUtilities::Vector2f& outScreenPosition) const;

private:
	void UpdateFrustum() const;

	void UpdateFreeLook(float aDeltaTime);
	void UpdateFollowTarget(float aDeltaTime);
	void UpdateLookAtTarget(float aDeltaTime);

	const Actor* myTarget;

	std::unique_ptr<CameraData> myCameraData;

	CameraMode myMode = CameraMode::FreeLook;

	CommonUtilities::Vector3f myShakeOffset = { 0.f, 0.f, 0.f };

	float myShakeIntensity = 0.f;
	float myShakeDuration = 0.f;
	float myShakeTimer = 0.f;

	float myCameraMoveSpeed = 400.0f;
	float myMouseSensitivity = 0.2f;
	CommonUtilities::Vector2f myFollowOffset = { 300.0f, 300.0f };
	CommonUtilities::Vector2f myLookAtOffset = { -300.0f, 300.0f };

};
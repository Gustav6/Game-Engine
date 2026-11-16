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
	OrbitTarget,
};

class GameCamera : public Actor
{
public:
	GameCamera();

	void Init(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution);

	void Update(float aDeltaTime) override;

	void SetTarget(const Actor* aTarget);
	void SetMode(CameraMode aMode);

	const CameraData& GetCameraData() const;

	void FocusOn(const Actor* aTarget);

	CommonUtilities::Ray<float> ScreenToWorldRay(const CommonUtilities::Vector2f& aScreenPosition) const;
	bool WorldToScreen(const CommonUtilities::Vector3f& aWorldPosition, CommonUtilities::Vector2f& outScreenPosition) const;

private:
	void UpdateFrustum() const;

	void UpdateFreeLook(float aDeltaTime);
	void UpdateFollowTarget(float aDeltaTime);
	void UpdateOrbitTarget(float aDeltaTime);

	const Actor* myTarget;

	std::unique_ptr<CameraData> myCameraData;

	CameraMode myMode = CameraMode::FreeLook;

	float myYaw;
	float myPitch;

	float myCameraMoveSpeed = 400.0f;
	float myMouseSensitivity = 0.2f;
	CommonUtilities::Vector3f myOrbitOffset = { 300.0f, 300.0f, -300.0f };
	CommonUtilities::Vector3f myFollowOffset = { 300.0f, 300.0f, 300.0f };
	CommonUtilities::Vector3f myLastTargetPosition;

	bool myIsDirty;
};
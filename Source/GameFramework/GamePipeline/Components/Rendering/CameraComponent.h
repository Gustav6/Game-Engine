#pragma once

#include "GamePipeline/Components/Base/Component.h"

#include "CommonUtilities/Math/Vector2.hpp"

enum class CameraMode
{
    FollowTarget,
    FreeLook
};

class Camera;

class CameraComponent : public Component
{
public:
	CameraComponent(Actor* aOwner);

    void Init(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution);

    void Update(float aDeltaTime) override;

    void SetTarget(const Actor* aTarget);
    void SetMode(CameraMode aMode);

    CameraMode GetMode() const;
    const Camera& GetRenderCamera() const;

private:
	void UpdateFrustum() const;
	void UpdateFreeLook(float aDeltaTime) const;
	void UpdateFollowTarget(float aDeltaTime) const;

    const Actor* myTarget;

    std::unique_ptr<Camera> myRenderCamera;

    CameraMode myMode = CameraMode::FreeLook;

    float myCameraMoveSpeed = 400.0f;
    float myMouseSensitivity = 0.2f;
    float myFollowDistance = 300.0f;
    float myFollowHeight = 100.0f;
};


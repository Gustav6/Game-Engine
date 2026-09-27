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

	void Update(float aDeltaTime) override;

	const Camera& GetRenderCamera() const;

private:
	GameCamera() = default;

	void UpdateFrustum();

	std::unique_ptr<Camera> myCamera;

	float myCamerasMoveSpeed;
};
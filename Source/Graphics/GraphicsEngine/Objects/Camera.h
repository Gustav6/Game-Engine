#pragma once

#include "CommonUtilities/Vector2.hpp"
#include "CommonUtilities/Matrix4x4.hpp"
#include "CommonUtilities/Frustum.hpp"

class Camera
{
public:
	Camera();
	Camera(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution);

	void UpdateFrustum(const CommonUtilities::Matrix4x4f& aWorldTransform);

	const CommonUtilities::Matrix4x4f& GetProjection() const;
	const CommonUtilities::Frustum<float>& GetFrustum() const;

	float GetNearPlane() const;
	float GetFarPlane() const;
	float GetFov() const;
	float GetAspectRatio() const;
	CommonUtilities::Vector2ui GetResolution() const;

private:
	CommonUtilities::Matrix4x4f myProjection;
	CommonUtilities::Frustum<float> myFrustum;

	float myNearPlane;
	float myFarPlane;

	CommonUtilities::Vector2ui myResolution;
};
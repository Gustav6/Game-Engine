#pragma once

#include "CommonUtilities/Math/Vector2.hpp"
#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Collision/Frustum.hpp"

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
	float GetFovInDegrees() const;
	float GetFovInRadians() const;
	float GetAspectRatio() const;
	CommonUtilities::Vector2ui GetResolution() const;

private:
	CommonUtilities::Matrix4x4f myProjection;
	CommonUtilities::Frustum<float> myFrustum;

	float myNearPlane;
	float myFarPlane;
	float myFovInDegrees;

	CommonUtilities::Vector2ui myResolution;
};
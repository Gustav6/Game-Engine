#include "GraphicsEngine.pch.h"
#include "Camera.h"

#include <cassert>
#include <cmath>

#include "CommonUtilities/MathUtils.hpp"

Camera::Camera() = default;

Camera::Camera(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution)
{
    assert(aNearPlane > 0.0f && aFarPlane > aNearPlane);

	myFarPlane = aFarPlane;
	myNearPlane = aNearPlane;

	myResolution = aResolution;

	float aspectRatio = static_cast<float>(aResolution.x) / static_cast<float>(aResolution.y);

	myFrustum = CommonUtilities::Frustum<float>(aHorizontalFov, aNearPlane, aFarPlane, aspectRatio);
    myProjection = CommonUtilities::Matrix4x4f::CreatePerspectiveProjection(aHorizontalFov, aNearPlane, aFarPlane, aResolution.x, aResolution.y);
}

void Camera::UpdateFrustum(const CommonUtilities::Matrix4x4f& aWorldTransform)
{
	myFrustum.BuildFrustum(aWorldTransform);
}

const CommonUtilities::Matrix4x4f& Camera::GetProjection() const
{
	return myProjection;
}

const CommonUtilities::Frustum<float>& Camera::GetFrustum() const
{
	return myFrustum;
}

float Camera::GetNearPlane() const
{
	return myNearPlane;
}

float Camera::GetFarPlane() const
{
	return myFarPlane;
}

float Camera::GetFov() const
{
	return myFrustum.GetFov();
}

float Camera::GetAspectRatio() const
{
	return myFrustum.GetAspectRatio();
}

CommonUtilities::Vector2ui Camera::GetResolution() const
{
	return myResolution;
}

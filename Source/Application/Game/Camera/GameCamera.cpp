#include "GameCamera.h"

#include "GraphicsEngine/Objects/Camera.h"

GameCamera::GameCamera() = default;

GameCamera::GameCamera(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution)
{ 
	myCamera = std::make_unique<Camera>(aHorizontalFov, aNearPlane, aFarPlane, aResolution);
}

void GameCamera::UpdateFrustum()
{
	myCamera->UpdateFrustum(GetTransform().GetWorldMatrix());
}

const Camera& GameCamera::GetCamera() const
{
	return *myCamera;
}

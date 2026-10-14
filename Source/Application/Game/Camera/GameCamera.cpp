#include "GameCamera.h"

#include "GraphicsEngine/Objects/Scene/Camera.h"

#include "Application.h"


GameCamera& GameCamera::GetInstance()
{
	static GameCamera instance;
	return instance;
}

void GameCamera::Init(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution)
{
	myCamera = std::make_unique<Camera>(aHorizontalFov, aNearPlane, aFarPlane, aResolution);
	myCamera->UpdateFrustum(GetTransform().GetWorldMatrix());
	myCamerasMoveSpeed = 400;
}

void GameCamera::Update(float aDeltaTime)
{
	if (Application.InputManager.IsKeyDown(KeyCode::W))
	{
		GetTransform().Translate(GetTransform().GetForward() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (Application.InputManager.IsKeyDown(KeyCode::A))
	{
		GetTransform().Translate(-GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (Application.InputManager.IsKeyDown(KeyCode::S))
	{
		GetTransform().Translate(-GetTransform().GetForward() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (Application.InputManager.IsKeyDown(KeyCode::D))
	{
		GetTransform().Translate(GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (Application.InputManager.IsKeyDown(KeyCode::Space))
	{
		GetTransform().Translate(GetTransform().GetUp() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (Application.InputManager.IsKeyDown(KeyCode::Control))
	{
		GetTransform().Translate(-GetTransform().GetUp() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (Application.InputManager.IsKeyPressed(KeyCode::RMB))
	{
		Application.InputManager.CaptureMouse();
		Application.InputManager.HideMouse();
	}
	else if (Application.InputManager.IsKeyReleased(KeyCode::RMB))
	{
		Application.InputManager.ReleaseMouse();
		Application.InputManager.ShowMouse();
	}

	if (Application.InputManager.IsKeyDown(KeyCode::RMB))
	{
		constexpr float sensitivity = 0.2f;

		CommonUtilities::Vector2f mouseDelta = Application.InputManager.GetMousePositionDelta();

		float yaw = mouseDelta.x * sensitivity;
		float pitch = mouseDelta.y * sensitivity;

		CommonUtilities::Quaternion<float> pitchQuat({ 1, 0, 0 }, pitch);
		CommonUtilities::Quaternion<float> yawQuat({ 0, 1, 0 }, yaw);

		GetTransform().Rotate(pitchQuat, false);
		GetTransform().Rotate(yawQuat, true);
		UpdateFrustum();
	}
}

const Camera& GameCamera::GetRenderCamera() const
{
	return *myCamera;
}

void GameCamera::UpdateFrustum()
{
	myCamera->UpdateFrustum(GetTransform().GetWorldMatrix());
}
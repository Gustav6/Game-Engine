#include "GameCamera.h"

#include "GraphicsEngine/Objects/Scene/Camera.h"

#include "EngineContext.h"


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
	if (EngineContext.InputManager.IsKeyDown(KeyCode::W))
	{
		GetTransform().Translate(GetTransform().GetForward() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (EngineContext.InputManager.IsKeyDown(KeyCode::A))
	{
		GetTransform().Translate(-GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (EngineContext.InputManager.IsKeyDown(KeyCode::S))
	{
		GetTransform().Translate(-GetTransform().GetForward() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (EngineContext.InputManager.IsKeyDown(KeyCode::D))
	{
		GetTransform().Translate(GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (EngineContext.InputManager.IsKeyDown(KeyCode::Space))
	{
		GetTransform().Translate(GetTransform().GetUp() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (EngineContext.InputManager.IsKeyDown(KeyCode::Control))
	{
		GetTransform().Translate(-GetTransform().GetUp() * myCamerasMoveSpeed * aDeltaTime);
		UpdateFrustum();
	}

	if (EngineContext.InputManager.IsKeyPressed(KeyCode::RMB))
	{
		EngineContext.InputManager.CaptureMouse();
		EngineContext.InputManager.HideMouse();
		EngineContext.InputManager.CenterCursor();
	}
	else if (EngineContext.InputManager.IsKeyReleased(KeyCode::RMB))
	{
		EngineContext.InputManager.ReleaseMouse();
		EngineContext.InputManager.ShowMouse();
	}

	if (EngineContext.InputManager.IsKeyDown(KeyCode::RMB))
	{
		const CommonUtilities::Vector2f mouseDelta = EngineContext.InputManager.GetCapturedMouseDelta();

		constexpr float sensitivity = 0.2f;

		float yaw = mouseDelta.x * sensitivity;
		float pitch = mouseDelta.y * sensitivity;

		CommonUtilities::Quaternion<float> pitchQuat({ 1, 0, 0 }, pitch);
		CommonUtilities::Quaternion<float> yawQuat({ 0, 1, 0 }, yaw);

		GetTransform().Rotate(pitchQuat, false);
		GetTransform().Rotate(yawQuat, true);
		UpdateFrustum();

		EngineContext.InputManager.CenterCursor();
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
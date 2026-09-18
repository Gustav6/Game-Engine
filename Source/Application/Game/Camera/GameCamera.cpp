#include "GameCamera.h"

#include "GraphicsEngine/Objects/Camera.h"

#include "Application.h"


GameCamera::GameCamera(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution) : myMode(Mode::FreeLook)
{ 
	myCamera = std::make_unique<Camera>(aHorizontalFov, aNearPlane, aFarPlane, aResolution);
	myCamerasMoveSpeed = 400;
}

GameCamera& GameCamera::GetInstance()
{
	static GameCamera instance;
	return instance;
}

void GameCamera::Init(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution)
{
	myCamera = std::make_unique<Camera>(aHorizontalFov, aNearPlane, aFarPlane, aResolution);
	myMode = Mode::FreeLook;
	myCamerasMoveSpeed = 400;
}

void GameCamera::Update(float aDeltaTime, Transform& aPlyerTransform)
{

	if (Application.InputManager.IsKeyDown(KeyCode::B))
	{
		myMode = Mode::FreeLook;
	}
	else if (Application.InputManager.IsKeyDown(KeyCode::N))
	{
		myMode = Mode::LookAt;
	}
	else if (Application.InputManager.IsKeyDown(KeyCode::M))
	{
		myMode = Mode::LookAtLocked;
	}

	if (myMode == Mode::LookAt)
	{	
		UpdateLockAt(aPlyerTransform, aDeltaTime);
		
	}
	else if (myMode == Mode::FreeLook)
	{
		UpdateFreeLock(aDeltaTime);
	}
	else if (myMode == Mode::LookAtLocked)
	{
		UpdateLockAtLocked(aPlyerTransform, aDeltaTime);
	}



	
}

void GameCamera::UpdateFrustum()
{
	myCamera->UpdateFrustum(GetTransform().GetWorldMatrix());
}

void GameCamera::UpdateMovment(float aDeltaTime)
{
	aDeltaTime;
	if (Application.InputManager.GetScrollWheelDelta() > 0)
	{
		myCamerasMoveSpeed *= 1.05f;
	}
	else if (Application.InputManager.GetScrollWheelDelta() < 0)
	{
		myCamerasMoveSpeed *= 0.95f;
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
}

void GameCamera::UpdateLockAt(Transform& aPlyerTransform, float aDeltaTime)
{
	if (Application.InputManager.IsKeyDown(KeyCode::W))
	{
		GetTransform().Translate(GetTransform().GetForward() * myCamerasMoveSpeed *	aDeltaTime);
	}

	if (Application.InputManager.IsKeyDown(KeyCode::A))
	{
		GetTransform().Translate(-GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime);
	}

	if (Application.InputManager.IsKeyDown(KeyCode::S))
	{
		GetTransform().Translate(-GetTransform().GetForward() *	myCamerasMoveSpeed * aDeltaTime);
	}

	if (Application.InputManager.IsKeyDown(KeyCode::D))
	{
		GetTransform().Translate(GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime);
	}

	if (Application.InputManager.IsKeyDown(KeyCode::Space))
	{
		GetTransform().Translate(GetTransform().GetUp() * myCamerasMoveSpeed * aDeltaTime);
	}

	if (Application.InputManager.IsKeyDown(KeyCode::Control))
	{
		GetTransform().Translate(-GetTransform().GetUp() *	myCamerasMoveSpeed * aDeltaTime);
	}
	CommonUtilities::Vector3f cameraPosition =	GetTransform().GetPosition();

	CommonUtilities::Vector3f direction = aPlyerTransform.GetPosition() - cameraPosition;

	direction.Normalize();

	float yaw = std::atan2(direction.x, direction.z);
	float horizontal = std::sqrt(direction.x * direction.x + direction.z * direction.z);
	float pitch = std::atan2(direction.y, horizontal);


	GetTransform().SetEulerRotation(CommonUtilities::Vector3f(pitch ,yaw ,0.0f ));

	UpdateFrustum();

}

void GameCamera::UpdateLockAtLocked(Transform& aPlyerTransform, float aDeltaTime)
{
	if (Application.InputManager.IsKeyDown(KeyCode::W))
	{
		GetTransform().Translate(GetTransform().GetForward() * myCamerasMoveSpeed * aDeltaTime);
	}

	if (Application.InputManager.IsKeyDown(KeyCode::A))
	{
		GetTransform().Translate(-GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime);
	}

	if (Application.InputManager.IsKeyDown(KeyCode::S))
	{
		GetTransform().Translate(-GetTransform().GetForward() * myCamerasMoveSpeed * aDeltaTime);
	}

	if (Application.InputManager.IsKeyDown(KeyCode::D))
	{
		GetTransform().Translate(GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime);
	}

	if (Application.InputManager.IsKeyDown(KeyCode::Space))
	{
		GetTransform().Translate(GetTransform().GetUp() * myCamerasMoveSpeed * aDeltaTime);
	}

	if (Application.InputManager.IsKeyDown(KeyCode::Control))
	{
		GetTransform().Translate(-GetTransform().GetUp() * myCamerasMoveSpeed * aDeltaTime);
	}

	const float cameraDistance = 1000.0f;
	const float cameraHeight = 1000.0f;

	const CommonUtilities::Vector3f playerPosition = aPlyerTransform.GetPosition();

	const CommonUtilities::Vector3f playerForward =	aPlyerTransform.GetForward();

	const CommonUtilities::Vector3f playerUp =	aPlyerTransform.GetUp();

	
	CommonUtilities::Vector3f cameraPosition = playerPosition	- playerForward * cameraDistance + playerUp * cameraHeight;

	GetTransform().SetPosition(cameraPosition);

	
	CommonUtilities::Vector3f direction = playerPosition - cameraPosition;

	direction.Normalize();

	const float yaw = std::atan2(direction.x, direction.z);

	const float horizontalLength = std::sqrt(direction.x * direction.x +	direction.z * direction.z);

	const float pitch =	std::atan2(direction.y, horizontalLength);

	GetTransform().SetEulerRotation(CommonUtilities::Vector3f(pitch, yaw,0.0f));

	UpdateFrustum();
}

void GameCamera::UpdateFreeLock(float aDeltaTime)
{
	if (Application.InputManager.IsKeyDown(KeyCode::W))
	{
		CommonUtilities::Vector3f delta = GetTransform().GetForward() * myCamerasMoveSpeed * aDeltaTime;

		GetTransform().Translate(delta);
		UpdateFrustum();
	}
	if (Application.InputManager.IsKeyDown(KeyCode::A))
	{
		CommonUtilities::Vector3f delta = -GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime;

		GetTransform().Translate(delta);
		UpdateFrustum();
	}
	if (Application.InputManager.IsKeyDown(KeyCode::S))
	{
		CommonUtilities::Vector3f delta = -GetTransform().GetForward() * myCamerasMoveSpeed * aDeltaTime;

		GetTransform().Translate(delta);
		UpdateFrustum();
	}
	if (Application.InputManager.IsKeyDown(KeyCode::D))
	{
		CommonUtilities::Vector3f delta = GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime;

		GetTransform().Translate(delta);
		UpdateFrustum();
	}

	if (Application.InputManager.IsKeyDown(KeyCode::Space))
	{
		CommonUtilities::Vector3f delta = GetTransform().GetUp() * myCamerasMoveSpeed * aDeltaTime;

		GetTransform().Translate(delta);
		UpdateFrustum();
	}
	if (Application.InputManager.IsKeyDown(KeyCode::Control))
	{
		CommonUtilities::Vector3f delta = -GetTransform().GetUp() * myCamerasMoveSpeed * aDeltaTime;

		GetTransform().Translate(delta);
		UpdateFrustum();
	}

	if (Application.InputManager.IsKeyDown(KeyCode::RMB))
	{
		const float sensitivity = 0.2f;

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

Mode GameCamera::GetMode() const
{
	return myMode;
}

const Camera& GameCamera::GetCamera() const
{
	return *myCamera;
}

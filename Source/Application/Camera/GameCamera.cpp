#include "GameCamera.h"

#include "Core/EngineContext.h"

#include "Wrappers/Logger/LoggerWrapper.h"

#include "Graphics/Objects/Scene/Camera.h"

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Quaternion.hpp"

GameCamera& GameCamera::GetInstance()
{
	static GameCamera instance;
	return instance;
}

void GameCamera::Init(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution)
{
	myRenderCamera = std::make_unique<Camera>(aHorizontalFov, aNearPlane, aFarPlane, aResolution);
	myTarget = nullptr;

	UpdateFrustum();
}

void GameCamera::Update(float aDeltaTime)
{
	if (myRenderCamera == nullptr)
	{
		SPDLOG_WARN("Camera Component has no render camera");
		return;
	}

	switch (myMode)
	{
		case CameraMode::FreeLook:
		{
			UpdateFreeLook(aDeltaTime);
			break;
		}
		case CameraMode::FollowTarget:
		{
			UpdateFollowTarget(aDeltaTime);
			break;
		}
	}

	UpdateFrustum();
}

void GameCamera::SetTarget(const Actor* aTarget)
{
	myTarget = aTarget;
}

void GameCamera::SetMode(CameraMode aMode)
{
	myMode = aMode;

	if (myMode == CameraMode::FollowTarget)
	{
		EngineContext.InputManager.ReleaseMouse();
		EngineContext.InputManager.ShowMouse();
	}
}

const Camera& GameCamera::GetRenderCamera() const
{
	return *myRenderCamera;
}

CommonUtilities::Ray<float> GameCamera::ScreenToWorldRay(const CommonUtilities::Vector2f& aScreenPosition) const
{
	//convert screen pos to ndc 

	const float ndcX = (2.0f * (aScreenPosition.x / static_cast<float>(myRenderCamera->GetResolution().x))) - 1.0f;

	const float ndcY = 1.0f - (2.0f * (aScreenPosition.y / static_cast<float>(myRenderCamera->GetResolution().y)));

	// Position on the far plane in clip space
	CommonUtilities::Vector4f clipPosition =
	{
		ndcX,
		ndcY,
		0.f,
		1.0f
	};

	// Clip space -> view space
	const CommonUtilities::Matrix4x4f inverseProjection = myRenderCamera->GetProjection().GetInverse();

	CommonUtilities::Vector4f viewPosition = clipPosition * inverseProjection;

	// Perspective divide
	if (viewPosition.w != 0.0f)
	{
		viewPosition.x /= viewPosition.w;
		viewPosition.y /= viewPosition.w;
		viewPosition.z /= viewPosition.w;
		viewPosition.w = 1.0f;
	}
	CommonUtilities::Vector4f worldPosition = viewPosition * GetTransform().GetWorldMatrix();

	const CommonUtilities::Vector3f rayOrigin = GetTransform().GetPosition();

	const CommonUtilities::Vector3f rayDirection = CommonUtilities::Vector3f(worldPosition.x - rayOrigin.x, worldPosition.y - rayOrigin.y, worldPosition.z - rayOrigin.z).GetNormalized();

	return CommonUtilities::Ray<float>(rayOrigin, rayDirection);
}

bool GameCamera::WorldToScreen(const CommonUtilities::Vector3f& aWorldPosition, CommonUtilities::Vector2f& outScreenPosition) const
{
	CommonUtilities::Vector4f worldPosition =
	{
		aWorldPosition.x,
		aWorldPosition.y,
		aWorldPosition.z,
		1.0f
	};

	const CommonUtilities::Matrix4x4f viewMatrix = GetTransform().GetWorldMatrix().GetFastInverse();
	const CommonUtilities::Matrix4x4f& projectionMatrix = myRenderCamera->GetProjection();

	CommonUtilities::Vector4f clipPosition = worldPosition * viewMatrix * projectionMatrix;

	if (clipPosition.w <= 0.0f)
	{
		return false;
	}


	// Perspective divide 
	const float ndcX = clipPosition.x / clipPosition.w;
	const float ndcY = clipPosition.y / clipPosition.w;


	if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f)
	{
		return false;
	}

	const CommonUtilities::Vector2ui resolution = myRenderCamera->GetResolution();

	outScreenPosition.x = (ndcX + 1.0f) * 0.5f * static_cast<float>(resolution.x);
	outScreenPosition.y = (1.0f - ndcY) * 0.5f * static_cast<float>(resolution.y);
	return true;
}

void GameCamera::UpdateFrustum() const
{
	if (myRenderCamera == nullptr)
	{
		return;
	}

	myRenderCamera->UpdateFrustum(GetTransform().GetWorldMatrix());
}

void GameCamera::UpdateFreeLook(float aDeltaTime)
{
	if (EngineContext.InputManager.IsKeyDown(KeyCode::W))
	{
		GetTransform().Translate(GetTransform().GetForward() * myCameraMoveSpeed * aDeltaTime);
	}

	if (EngineContext.InputManager.IsKeyDown(KeyCode::A))
	{
		GetTransform().Translate(-GetTransform().GetRight() * myCameraMoveSpeed * aDeltaTime);
	}

	if (EngineContext.InputManager.IsKeyDown(KeyCode::S))
	{
		GetTransform().Translate(-GetTransform().GetForward() * myCameraMoveSpeed * aDeltaTime);
	}

	if (EngineContext.InputManager.IsKeyDown(KeyCode::D))
	{
		GetTransform().Translate(GetTransform().GetRight() * myCameraMoveSpeed * aDeltaTime);
	}

	if (EngineContext.InputManager.IsKeyDown(KeyCode::Space))
	{
		GetTransform().Translate(GetTransform().GetUp() * myCameraMoveSpeed * aDeltaTime);
	}

	if (EngineContext.InputManager.IsKeyDown(KeyCode::Control))
	{
		GetTransform().Translate(-GetTransform().GetUp() * myCameraMoveSpeed * aDeltaTime);
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

		float pitch = mouseDelta.y * sensitivity;
		float yaw = mouseDelta.x * sensitivity;

		CommonUtilities::Quaternion<float> pitchQuat({ 1.0f, 0.0f, 0.0f }, pitch);
		CommonUtilities::Quaternion<float> yawQuat({ 0.0f, 1.0f, 0.0f }, yaw);

		GetTransform().Rotate(pitchQuat, false);
		GetTransform().Rotate(yawQuat, true);

		EngineContext.InputManager.CenterCursor();
	}

	UpdateFrustum();
}

void GameCamera::UpdateFollowTarget(float aDeltaTime)
{
	aDeltaTime;

	if (myTarget == nullptr)
	{
		return;
	}

	const auto& targetTransform = myTarget->GetTransform();

	const auto targetPosition = targetTransform.GetPosition();
	const auto targetForward = targetTransform.GetForward();

	const auto cameraPosition = targetPosition - targetForward * myFollowDistance + targetTransform.GetUp() * myFollowHeight;

	GetTransform().SetPosition(cameraPosition);
}
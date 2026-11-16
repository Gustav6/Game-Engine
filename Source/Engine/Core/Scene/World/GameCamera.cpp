#include "Core/Core.pch.h"
#include "GameCamera.h"

#include "Core/EngineContext.h"

#include "Wrappers/Logger/LoggerWrapper.h"

#include <Core/Scene/Components/Concrete/Transform/Transform.h>

#include "Graphics/Objects/Scene/Camera.h"
#include "Graphics/Core/SceneCamera.h"

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Quaternion.hpp"

GameCamera::GameCamera() :
	Actor(0)
{
	myTarget = nullptr;
	myIsDirty = false;
}

void GameCamera::Init(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution)
{
	myCameraData = std::make_unique<CameraData>();
	myCameraData->RenderCamera.SetProjection(aHorizontalFov, aNearPlane, aFarPlane, aResolution);
	
	UpdateFrustum();
}

void GameCamera::Update(float aDeltaTime)
{
	if (myCameraData == nullptr)
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
		case CameraMode::OrbitTarget:
		{
			UpdateOrbitTarget(aDeltaTime);
			break;
		}
	}

	if (myIsDirty)
	{
		myCameraData->Transform = GetComponent<Transform>()->GetWorldMatrix();
		UpdateFrustum();
		myIsDirty = false;
	}
}

void GameCamera::SetTarget(const Actor* aTarget)
{
	myTarget = aTarget;
}

void GameCamera::SetMode(CameraMode aMode)
{
	myMode = aMode;

	GEngineContext.InputManager.ReleaseMouse();
	GEngineContext.InputManager.ShowMouse();
}

const CameraData& GameCamera::GetCameraData() const
{
	return *myCameraData;
}

void GameCamera::FocusOn(const Actor* aTarget)
{
	myTarget = aTarget;

	if (myTarget == nullptr)
	{
		return;
	}

	const CommonUtilities::Vector3f orbitCenter = myTarget->GetComponent<Transform>()->GetPosition();

	CommonUtilities::Matrix4x4f lookAtMatrix = CommonUtilities::Matrix4x4f::CreateLookAt(
		orbitCenter + myOrbitOffset,
		orbitCenter,
		CommonUtilities::Vector3f(0.0f, 1.0f, 0.0f)
	);

	GetComponent<Transform>()->SetWorldMatrix(lookAtMatrix);

	myIsDirty = true;

	SetMode(CameraMode::OrbitTarget);
}

CommonUtilities::Ray<float> GameCamera::ScreenToWorldRay(const CommonUtilities::Vector2f& aScreenPosition) const
{
	//convert screen pos to ndc 

	const float ndcX = (2.0f * (aScreenPosition.x / static_cast<float>(myCameraData->RenderCamera.GetResolution().x))) - 1.0f;
	const float ndcY = 1.0f - (2.0f * (aScreenPosition.y / static_cast<float>(myCameraData->RenderCamera.GetResolution().y)));

	// Position on the far plane in clip space
	CommonUtilities::Vector4f clipPosition =
	{
		ndcX,
		ndcY,
		0.f,
		1.0f
	};

	// Clip space -> view space
	const CommonUtilities::Matrix4x4f inverseProjection = myCameraData->RenderCamera.GetProjection().GetInverse();

	CommonUtilities::Vector4f viewPosition = clipPosition * inverseProjection;

	// Perspective divide
	if (viewPosition.w != 0.0f)
	{
		viewPosition.x /= viewPosition.w;
		viewPosition.y /= viewPosition.w;
		viewPosition.z /= viewPosition.w;
		viewPosition.w = 1.0f;
	}

	CommonUtilities::Vector4f worldPosition = viewPosition * GetComponent<Transform>()->GetWorldMatrix();

	const CommonUtilities::Vector3f rayOrigin = GetComponent<Transform>()->GetPosition();

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

	const CommonUtilities::Matrix4x4f viewMatrix = GetComponent<Transform>()->GetWorldMatrix().GetFastInverse();
	const CommonUtilities::Matrix4x4f& projectionMatrix = myCameraData->RenderCamera.GetProjection();

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

	const CommonUtilities::Vector2ui resolution = myCameraData->RenderCamera.GetResolution();

	outScreenPosition.x = (ndcX + 1.0f) * 0.5f * static_cast<float>(resolution.x);
	outScreenPosition.y = (1.0f - ndcY) * 0.5f * static_cast<float>(resolution.y);
	return true;
}

void GameCamera::UpdateFrustum() const
{
	if (myCameraData == nullptr)
	{
		return;
	}

	myCameraData->RenderCamera.UpdateFrustum(GetComponent<Transform>()->GetWorldMatrix());
}

void GameCamera::UpdateFreeLook(float aDeltaTime)
{
	if (GEngineContext.InputManager.IsKeyDown(KeyCode::W))
	{
		GetComponent<Transform>()->Translate(GetComponent<Transform>()->GetForward() * myCameraMoveSpeed * aDeltaTime);

		myIsDirty = true;
	}

	if (GEngineContext.InputManager.IsKeyDown(KeyCode::A))
	{
		GetComponent<Transform>()->Translate(-GetComponent<Transform>()->GetRight() * myCameraMoveSpeed * aDeltaTime);

		myIsDirty = true;
	}

	if (GEngineContext.InputManager.IsKeyDown(KeyCode::S))
	{
		GetComponent<Transform>()->Translate(-GetComponent<Transform>()->GetForward() * myCameraMoveSpeed * aDeltaTime);

		myIsDirty = true;
	}

	if (GEngineContext.InputManager.IsKeyDown(KeyCode::D))
	{
		GetComponent<Transform>()->Translate(GetComponent<Transform>()->GetRight() * myCameraMoveSpeed * aDeltaTime);

		myIsDirty = true;
	}

	if (GEngineContext.InputManager.IsKeyDown(KeyCode::Space))
	{
		GetComponent<Transform>()->Translate(GetComponent<Transform>()->GetUp() * myCameraMoveSpeed * aDeltaTime);

		myIsDirty = true;
	}

	if (GEngineContext.InputManager.IsKeyDown(KeyCode::Control))
	{
		GetComponent<Transform>()->Translate(-GetComponent<Transform>()->GetUp() * myCameraMoveSpeed * aDeltaTime);

		myIsDirty = true;
	}

	if (GEngineContext.InputManager.IsKeyPressed(KeyCode::RMB))
	{
		GEngineContext.InputManager.CaptureMouse();
		GEngineContext.InputManager.HideMouse();
		GEngineContext.InputManager.CenterCursor();
	}
	else if (GEngineContext.InputManager.IsKeyReleased(KeyCode::RMB))
	{
		GEngineContext.InputManager.ReleaseMouse();
		GEngineContext.InputManager.ShowMouse();
	}

	if (GEngineContext.InputManager.IsKeyDown(KeyCode::RMB))
	{
		const CommonUtilities::Vector2f mouseDelta = GEngineContext.InputManager.GetCapturedMouseDelta();

		float pitch = mouseDelta.y * myMouseSensitivity;
		float yaw = mouseDelta.x * myMouseSensitivity;

		CommonUtilities::Quaternion<float> pitchQuat({ 1.0f, 0.0f, 0.0f }, pitch);
		CommonUtilities::Quaternion<float> yawQuat({ 0.0f, 1.0f, 0.0f }, yaw);

		GetComponent<Transform>()->Rotate(pitchQuat, false);
		GetComponent<Transform>()->Rotate(yawQuat, true);

		GEngineContext.InputManager.CenterCursor();

		myIsDirty = true;
	}
}

void GameCamera::UpdateFollowTarget(float aDeltaTime)
{
	aDeltaTime;

	if (myTarget == nullptr)
	{
		return;
	}

	const auto& targetTransform = myTarget->GetComponent<Transform>();

	const auto targetPosition = targetTransform->GetPosition();

	if (targetPosition == myLastTargetPosition)
	{
		return;
	}

	myLastTargetPosition = targetPosition;

	const auto targetForward = targetTransform->GetForward();

	const auto cameraPosition = targetPosition - targetForward * myFollowOffset.x + targetTransform->GetUp() * myFollowOffset.y;

	GetComponent<Transform>()->SetPosition(cameraPosition);

	myIsDirty = true;
}

void GameCamera::UpdateOrbitTarget(float aDeltaTime)
{
	aDeltaTime;

	if (myTarget == nullptr)
	{
		return;
	}

	if (GEngineContext.InputManager.IsKeyPressed(KeyCode::RMB))
	{
		GEngineContext.InputManager.CaptureMouse();
		GEngineContext.InputManager.HideMouse();
		GEngineContext.InputManager.CenterCursor();
	}
	else if (GEngineContext.InputManager.IsKeyReleased(KeyCode::RMB))
	{
		GEngineContext.InputManager.ReleaseMouse();
		GEngineContext.InputManager.ShowMouse();
	}

	if (GEngineContext.InputManager.IsKeyDown(KeyCode::RMB))
	{
		const CommonUtilities::Vector2f mouseDelta = GEngineContext.InputManager.GetCapturedMouseDelta();

		myYaw += mouseDelta.x * myMouseSensitivity;
		myPitch += mouseDelta.y * myMouseSensitivity;

		CommonUtilities::Quaternionf yawQuat({ 0.0f, 1.0f, 0.0f }, myYaw);
		CommonUtilities::Quaternionf pitchQuat({ 1.0f, 0.0f, 0.0f }, myPitch);
		CommonUtilities::Quaternionf rotation = (yawQuat * pitchQuat).GetNormalized();

		const CommonUtilities::Vector3f orbitCenter = myTarget->GetComponent<Transform>()->GetPosition();

		const CommonUtilities::Vector3f orbitOffset =
			CommonUtilities::Quaternionf::RotateVectorByQuaternion(rotation, myOrbitOffset);

		CommonUtilities::Matrix4x4f lookAtMatrix = CommonUtilities::Matrix4x4f::CreateLookAt(
			orbitCenter + orbitOffset,
			orbitCenter,
			CommonUtilities::Vector3f(0.0f, 1.0f, 0.0f)
		);

		GetComponent<Transform>()->SetWorldMatrix(lookAtMatrix);

		GEngineContext.InputManager.CenterCursor();

		myIsDirty = true;
	}
}

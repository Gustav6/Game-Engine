#include "Core/Core.pch.h"
#include "GameCamera.h"

#include "Core/EngineContext.h"

#include "Wrappers/Logger/LoggerWrapper.h"

#include <Core/Scene/Components/Concrete/Transform/Transform.h>

#include "Graphics/Objects/Scene/Camera.h"
#include "Graphics/Core/SceneCamera.h"

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Quaternion.hpp"

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>

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
	ImGuiIO& imGuiIO = ImGui::GetIO();
	const bool imguiMouseFocused = imGuiIO.WantCaptureMouse && !ImGuizmo::IsOver();
	const bool imguiKeyboardFocused = imGuiIO.WantCaptureKeyboard;

	if (!imguiKeyboardFocused)
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
	}

	if (!imguiMouseFocused)
	{
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

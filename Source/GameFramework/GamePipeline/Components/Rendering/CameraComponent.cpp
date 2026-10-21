#include "GameFramework.pch.h"
#include "CameraComponent.h"

#include <Logger/Logger.h>

#include "GraphicsEngine/Objects/Scene/Camera.h"
#include "GamePipeline/World/Actor.hpp"

#include "EngineContext.h"

CameraComponent::CameraComponent(Actor* aOwner)
    : Component(aOwner)
{
	myTarget = nullptr;
}

void CameraComponent::Init(float aHorizontalFov, float aNearPlane, float aFarPlane, CommonUtilities::Vector2ui aResolution)
{
    myRenderCamera = std::make_unique<Camera>(aHorizontalFov, aNearPlane, aFarPlane, aResolution);

    UpdateFrustum();
}

void CameraComponent::Update(float aDeltaTime)
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

void CameraComponent::SetTarget(const Actor* aTarget)
{
    myTarget = aTarget;
}

void CameraComponent::SetMode(CameraMode aMode)
{
    myMode = aMode;

    if (myMode == CameraMode::FollowTarget)
    {
        EngineContext.InputManager.ReleaseMouse();
        EngineContext.InputManager.ShowMouse();
    }
}

CameraMode CameraComponent::GetMode() const
{
    return myMode;
}

const Camera& CameraComponent::GetRenderCamera() const
{
    return *myRenderCamera;
}

void CameraComponent::UpdateFreeLook(float aDeltaTime) const
{
    if (EngineContext.InputManager.IsKeyDown(KeyCode::W))
    {
        myOwner->GetTransform().Translate(myOwner->GetTransform().GetForward() * myCameraMoveSpeed * aDeltaTime);
    }

    if (EngineContext.InputManager.IsKeyDown(KeyCode::A))
    {
        myOwner->GetTransform().Translate(-myOwner->GetTransform().GetRight() * myCameraMoveSpeed * aDeltaTime);
    }

    if (EngineContext.InputManager.IsKeyDown(KeyCode::S))
    {
        myOwner->GetTransform().Translate(-myOwner->GetTransform().GetForward() * myCameraMoveSpeed * aDeltaTime);
    }

    if (EngineContext.InputManager.IsKeyDown(KeyCode::D))
    {
        myOwner->GetTransform().Translate(myOwner->GetTransform().GetRight() * myCameraMoveSpeed * aDeltaTime);
    }

    if (EngineContext.InputManager.IsKeyDown(KeyCode::Space))
    {
        myOwner->GetTransform().Translate(myOwner->GetTransform().GetUp() * myCameraMoveSpeed * aDeltaTime);
    }

    if (EngineContext.InputManager.IsKeyDown(KeyCode::Control))
    {
        myOwner->GetTransform().Translate(-myOwner->GetTransform().GetUp() * myCameraMoveSpeed * aDeltaTime);
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
        const auto mouseDelta = EngineContext.InputManager.GetMousePositionDelta();

        const float yaw = mouseDelta.x * myMouseSensitivity;
        const float pitch = mouseDelta.y * myMouseSensitivity;

        CommonUtilities::Quaternion<float> pitchQuat({ 1, 0, 0 }, pitch);
        CommonUtilities::Quaternion<float> yawQuat({ 0, 1, 0 }, yaw);

        myOwner->GetTransform().Rotate(pitchQuat, false);
        myOwner->GetTransform().Rotate(yawQuat, true);

        EngineContext.InputManager.CenterCursor();
    }
}

void CameraComponent::UpdateFollowTarget(float aDeltaTime) const
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

    myOwner->GetTransform().SetPosition(cameraPosition);
}

void CameraComponent::UpdateFrustum() const
{
    if (myRenderCamera)
    {
        myRenderCamera->UpdateFrustum(myOwner->GetTransform().GetWorldMatrix());
    }
}

#include "GameFramework.pch.h"
#include "SpotLightComponent.h"

#include <cassert>

#include "CommonUtilities/Math/Vector4.hpp"
#include "CommonUtilities/Math/MathUtils.hpp"

#include "GraphicsEngine/Objects/Scene/Light.h"

#include "GraphicsEngine/Drawers/LightDrawer.h"

SpotLightComponent::SpotLightComponent(Actor* aOwner)
	: LightComponent(aOwner)
{
	myData->Type = LightType::Spot;
}

void SpotLightComponent::Render()
{
	UpdateLightViewProjection();

	SceneLight light =
	{
	    .LightData = myData.get(),
	    .ShadowMap = myShadowMap.get(),
	    .Transform = GetTransform(),
	    .Projection = myProjection
	};

	LightDrawer::Get().Submit(light);
}

void SpotLightComponent::SetDirection(const CommonUtilities::Vector3f & aDirection)
{
	myData->Direction = aDirection.GetNormalized();

    CommonUtilities::Vector3f forward = myData->Direction;

    CommonUtilities::Vector3f worldUp = { 0.0f, 1.0f, 0.0f };

    if (abs(CommonUtilities::Vector3f::Dot(forward, worldUp)) > 0.99f)
    {
        worldUp = { 1.0f, 0.0f, 0.0f };
    }

    CommonUtilities::Vector3f right = CommonUtilities::Vector3f::Cross(worldUp, forward).GetNormalized();
    CommonUtilities::Vector3f up = CommonUtilities::Vector3f::Cross(forward, right);

    myTransform(1, 1) = right.x;
    myTransform(1, 2) = right.y;
    myTransform(1, 3) = right.z;

    myTransform(2, 1) = up.x;
    myTransform(2, 2) = up.y;
    myTransform(2, 3) = up.z;

    myTransform(3, 1) = forward.x;
    myTransform(3, 2) = forward.y;
    myTransform(3, 3) = forward.z;
}

void SpotLightComponent::SetRadius(float aRadius)
{
	myData->Radius = aRadius;
}

void SpotLightComponent::SetCone(float aInner, float aOuter)
{
    SetInnerCone(aInner);
    SetOuterCone(aOuter);
}

void SpotLightComponent::SetInnerCone(float aInnerCone)
{
    assert(aInnerCone >= 0.0f && aInnerCone <= 1.0f);
    assert(aInnerCone <= std::acos(myData->CosOuterCone));

    myData->CosInnerCone = std::cos(aInnerCone);
}

void SpotLightComponent::SetOuterCone(float aOuterCone)
{
    assert(aOuterCone >= 0.0f && aOuterCone <= 1.0f);
    assert(std::acos(myData->CosInnerCone) <= aOuterCone);

    myData->CosOuterCone = std::cos(aOuterCone);
}

const CommonUtilities::Matrix4x4f SpotLightComponent::CreateProjectionMatrix() const
{
    const float fov = acos(myData->CosOuterCone) * 2.0f * CommonUtilities::Math::RAD_TO_DEG<float>;

	return CommonUtilities::Matrix4x4f::CreatePerspectiveProjection(fov, 0.1f, myData->Radius, 1.0f);
}

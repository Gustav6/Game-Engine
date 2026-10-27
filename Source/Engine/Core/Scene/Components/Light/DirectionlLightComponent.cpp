#include "Core.pch.h"
#include "DirectionlLightComponent.h"

#include "Graphics/Objects/Scene/Light.h"

#include "Graphics/Drawers/LightDrawer.h"

DirectionalLightComponent::DirectionalLightComponent(Actor* aOwner)
	: LightComponent(aOwner)
{
	myData.Type = LightType::Directional;
}

void DirectionalLightComponent::Render()
{
	UpdateLightViewProjection();

	SceneLight light =
	{
	    .LightData = &myData,
	    .Transform = GetTransform(),
	    .Projection = myProjection
	};

	LightDrawer::Get().Submit(light);
}

void DirectionalLightComponent::SetDirection(const CommonUtilities::Vector3f& aDirection)
{
	myData.Direction = aDirection.GetNormalized();

    CommonUtilities::Vector3f forward = myData.Direction;

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

const CommonUtilities::Matrix4x4f DirectionalLightComponent::CreateProjectionMatrix() const
{
	return CommonUtilities::Matrix4x4f::CreateOrthographicProjection(1.0f, 5000.0f, static_cast<float>(myData.ShadowMapSize.x), static_cast<float>(myData.ShadowMapSize.y));
}

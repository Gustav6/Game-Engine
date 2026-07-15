#include "PointLightComponent.h"

#include "CommonUtilities/Vector4.hpp"

#include "GraphicsEngine/Objects/Light.h"

PointLightComponent::PointLightComponent(GameObject* aOwner)
	: LightComponent(aOwner)
{
	myData->Type = LightType::Point;
}

void PointLightComponent::SetRadius(float aRadius)
{
	myData->Radius = aRadius;
}

const CommonUtilities::Matrix4x4f PointLightComponent::CreateProjectionMatrix() const
{
	return CommonUtilities::Matrix4x4f::CreatePerspectiveProjection(90, 1.0f, myData->Radius, myShadowMapSize.x, myShadowMapSize.y);
}

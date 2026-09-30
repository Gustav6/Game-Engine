#include "GameFramework.pch.h"
#include "PointLightComponent.h"

#include "CommonUtilities/Math/Vector4.hpp"

#include "GraphicsEngine/Objects/Scene/Light.h"

#include "GraphicsEngine/Drawers/LightDrawer.h"

PointLightComponent::PointLightComponent(Actor* aOwner)
	: LightComponent(aOwner)
{
	myData.Type = LightType::Point;
}

void PointLightComponent::Render()
{
	SceneLight light =
	{
		.LightData = &myData,
		.ShadowMap = myShadowMap.get(),
		.Transform = GetTransform(),
		.Projection = myProjection
	};

	LightDrawer::Get().Submit(light);
}

void PointLightComponent::SetRadius(float aRadius)
{
	myData.Radius = aRadius;
}

const CommonUtilities::Matrix4x4f PointLightComponent::CreateProjectionMatrix() const
{
	return CommonUtilities::Matrix4x4f::CreatePerspectiveProjection(90, 1.0f, myData.Radius, 1.0f);
}

#include "GameFramework.pch.h"
#include "PointLightComponent.h"

#include "CommonUtilities/Math/Vector4.hpp"

#include "GraphicsEngine/Objects/Scene/Light.h"

#include "GraphicsEngine/Drawers/LightDrawer.h"
#include "GraphicsEngine/Drawers/DebugDrawer.h"

#include "Managers/DebugSettingsManager.h"

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


#ifndef _RETAIL
	if (DebugSettingsManager::Get().GetSettings().LightGizmosDebugDrawEnabled)
	{
		DebugDrawer::Get().DrawCross(25, myTransform.GetTranslation(), DebugSettingsManager::Get().GetSettings().LightGizmoColor);
		DebugDrawer::Get().DrawSphere(myData.Radius, myTransform.GetTranslation(), DebugSettingsManager::Get().GetSettings().LightGizmoColor);
	}
#endif
}

void PointLightComponent::SetRadius(float aRadius)
{
	myData.Radius = aRadius;
}

const CommonUtilities::Matrix4x4f PointLightComponent::CreateProjectionMatrix() const
{
	return CommonUtilities::Matrix4x4f::CreatePerspectiveProjection(90, 1.0f, myData.Radius, 1.0f);
}

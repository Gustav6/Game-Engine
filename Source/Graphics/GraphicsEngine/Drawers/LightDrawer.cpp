#include "GraphicsEngine.pch.h"
#include "LightDrawer.h"

#include "GraphicsEngine/Objects/Scene/Light.h"

LightDrawer& LightDrawer::Get()
{
	static LightDrawer myInstance;
	return myInstance;
}

void LightDrawer::Submit(const SceneLight& aSceneLight)
{
	// A Directional Light gets sumbited without a shadow map since it uses cascades!

	if (aSceneLight.LightData == nullptr || aSceneLight.LightData->Type != LightType::Directional && aSceneLight.ShadowMap == nullptr)
	{
		GELOG(Error, "Light was submitted with invalid data!");
		return;
	}

	mySceneLights.emplace_back(aSceneLight);
}
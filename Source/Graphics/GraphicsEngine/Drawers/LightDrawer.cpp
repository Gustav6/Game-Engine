#include "LightDrawer.h"

LightDrawer& LightDrawer::Get()
{
	static LightDrawer myInstance;
	return myInstance;
}

void LightDrawer::Submit(const SceneLight& aSceneLight)
{
	mySceneLights.emplace_back(aSceneLight);
}
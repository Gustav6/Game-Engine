#pragma once

#include <Tools/Core/Interface/ITool.h>

class Actor;

class PropertiesPanel : public ITool
{
public:
	void Update(float aDeltaTime) override;
	void Render() override;

	void ShowProperties(const Actor* aActor);
};


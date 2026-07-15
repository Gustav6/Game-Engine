#pragma once

#include "Game/Component/Light/LightComponent.h"

class PointLightComponent : public LightComponent
{
public:
    PointLightComponent(GameObject* aOwner);

    void SetRadius(float aRadius);

protected:
    const CommonUtilities::Matrix4x4f CreateProjectionMatrix() const override;
};


#pragma once

#include "LightComponent.h"

class PointLightComponent : public LightComponent
{
public:
    PointLightComponent(Actor* aOwner);

    void Render() override;

    void SetRadius(float aRadius);

protected:
    const CommonUtilities::Matrix4x4f CreateProjectionMatrix() const override;
};


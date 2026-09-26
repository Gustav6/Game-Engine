#pragma once

#include "LightComponent.h"

class DirectionalLightComponent : public LightComponent
{
public:
    DirectionalLightComponent(Actor* aOwner);

    void Render() override;

    void SetDirection(const CommonUtilities::Vector3f& aDirection);

protected:
    const CommonUtilities::Matrix4x4f CreateProjectionMatrix() const override;
};


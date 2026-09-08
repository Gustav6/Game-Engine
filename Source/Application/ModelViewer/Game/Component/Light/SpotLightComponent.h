#pragma once

#include "Game/Component/Light/LightComponent.h"

class SpotLightComponent : public LightComponent
{
public:
    SpotLightComponent(GameObject* aOwner);

    void SetDirection(const CommonUtilities::Vector3f& aDirection);
    void SetRadius(float aRadius);
    void SetCone(float aInner, float aOuter);
    void SetInnerCone(float aInnerCone);
    void SetOuterCone(float aOuterCone);

protected:
    const CommonUtilities::Matrix4x4f CreateProjectionMatrix() const override;
};


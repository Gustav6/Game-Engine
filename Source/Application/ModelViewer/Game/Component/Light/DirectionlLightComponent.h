#pragma once

#include <vector>

#include "Game/Component/Light/LightComponent.h"

struct SceneShadowCascade;

class DirectionalLightComponent : public LightComponent
{
public:
    DirectionalLightComponent(GameObject* aOwner);

    void SetDirection(const CommonUtilities::Vector3f& aDirection);

protected:
    const CommonUtilities::Matrix4x4f CreateProjectionMatrix() const override;
};


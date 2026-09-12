#pragma once

#include <memory>

#include "Game/Components/Component.h"

#include "CommonUtilities/Vector2.hpp"
#include "CommonUtilities/Vector3.hpp"
#include "CommonUtilities/Matrix4x4.hpp"

struct Light;
class Texture;

class LightComponent : public Component
{
public:
    LightComponent(Actor* aOwner);
    virtual ~LightComponent();

    void UpdateLightViewProjection();

    const Light* GetData() const;

    void SetPosition(const CommonUtilities::Vector3f& aPosition);
    void SetColor(const CommonUtilities::Vector3f& aColor);
    virtual void SetIntensity(float aIntensity);

    void SetShadowMapSize(const CommonUtilities::Vector2ui& aSize);
    const Texture* GetShadowMap() const;

    const CommonUtilities::Matrix4x4f& GetProjection() const;
    const CommonUtilities::Matrix4x4f& GetTransform() const;

protected:
    virtual const CommonUtilities::Matrix4x4f CreateProjectionMatrix() const = 0;

    bool myTransformDirty;

    std::unique_ptr<Texture> myShadowMap;

    CommonUtilities::Matrix4x4f myProjectionMatrix;

    std::unique_ptr<Light> myData;

    CommonUtilities::Matrix4x4f myTransform;
};
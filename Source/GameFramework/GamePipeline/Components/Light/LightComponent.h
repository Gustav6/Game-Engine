#pragma once

#include <memory>

#include "GamePipeline/Components/Base/Component.h"

#include "CommonUtilities/Math/Vector2.hpp"
#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Math/Matrix4x4.hpp"

#include "GraphicsEngine/Objects/Scene/Light.h"

class Texture;

class LightComponent : public Component
{
public:
    LightComponent(Actor* aOwner);
    virtual ~LightComponent();

    LightComponent(const LightComponent&) = delete;
    LightComponent& operator=(const LightComponent&) = delete;

    LightComponent(LightComponent&&) noexcept = default;
    LightComponent& operator=(LightComponent&&) noexcept = default;

    void UpdateLightViewProjection();

    const Light& GetData() const;

    void SetPosition(const CommonUtilities::Vector3f& aPosition);
    void SetColor(const CommonUtilities::Vector3f& aColor);
    virtual void SetIntensity(float aIntensity);

    void SetShadowMapSize(const CommonUtilities::Vector2ui& aSize);
    const Texture* GetShadowMap() const;

    const CommonUtilities::Matrix4x4f& GetProjection() const;
    const CommonUtilities::Matrix4x4f& GetTransform() const;

protected:
    virtual const CommonUtilities::Matrix4x4f CreateProjectionMatrix() const = 0;

    std::unique_ptr<Texture> myShadowMap;

    Light myData;

    CommonUtilities::Matrix4x4f myProjection;
    CommonUtilities::Matrix4x4f myTransform;

    bool myTransformDirty;
};
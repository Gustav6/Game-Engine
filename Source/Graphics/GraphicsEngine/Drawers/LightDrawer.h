#pragma once

#include <vector>

#include <CommonUtilities/Math/Matrix4x4.hpp>

struct Light;
class Texture;

struct SceneLight
{
    const Light* LightData = nullptr;
    const Texture* ShadowMap = nullptr;
    CommonUtilities::Matrix4x4f Transform;
    CommonUtilities::Matrix4x4f Projection;
};

class LightDrawer
{
    friend class GraphicsEngine;

public:
    static LightDrawer& Get();

    void Submit(const SceneLight& aSceneLight);

private:
    std::vector<SceneLight> mySceneLights;
};
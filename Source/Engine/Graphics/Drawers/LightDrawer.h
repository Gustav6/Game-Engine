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
public:
    static LightDrawer& Get();

    void Submit(const SceneLight& aSceneLight);

    const std::vector<SceneLight>& GetSceneLights() const;

    void Clear();

private:
    std::vector<SceneLight> mySceneLights;
};
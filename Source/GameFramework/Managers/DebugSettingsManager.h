#pragma once

#include "CommonUtilities/Math/Vector4.hpp"

struct DebugSettings
{
    CommonUtilities::Vector4f ColliderColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    CommonUtilities::Vector4f MeshBoundingBoxColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    CommonUtilities::Vector4f MeshBoundingSphereColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    CommonUtilities::Vector4f LightGizmoColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    bool ColliderDebugDrawEnabled = false;
    bool MeshDebugDrawBoxEnabled = false;
    bool MeshDebugDrawSphereEnabled = false;
    bool LightGizmosDebugDrawEnabled = false;
};

class DebugSettingsManager
{
public:
    static DebugSettingsManager& Get();

    void SetSettings(const DebugSettings& aSettings);
    const DebugSettings& GetSettings() const;

private:
    DebugSettings mySettings;
};
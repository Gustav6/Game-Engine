#pragma once

#include <array>

class RenderHardwareInterface;
class BuiltinAssets;
struct ShadowCascade;
struct SceneCamera;
enum class DebugRenderMode;

struct RenderContext
{
    RenderHardwareInterface* RHI = nullptr;
    const BuiltinAssets* BuiltinAssets = nullptr;
    const SceneCamera* Camera = nullptr;
    DebugRenderMode Mode = {};
    std::array<ShadowCascade, 4>& CascadeShadows; // NUM_CASCADES
};
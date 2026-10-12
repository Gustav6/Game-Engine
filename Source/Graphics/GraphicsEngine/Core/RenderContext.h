#pragma once

#include <array>

class RenderHardwareInterface;
class BuiltinAssets;
class ConstantBufferManager;

struct ShadowCascade;
struct SceneCamera;
struct GraphicsEngineRenderSettings;

enum class DebugRenderMode;
enum class ToneMapper;

struct RenderContext
{
    ConstantBufferManager* ConstantBuffers = nullptr;
    RenderHardwareInterface* RHI = nullptr;
    const BuiltinAssets* BuiltinAssets = nullptr;
    const SceneCamera* Camera = nullptr;
    GraphicsEngineRenderSettings* RenderSettings = {};
    const std::array<ShadowCascade, 4>* CascadeShadows = nullptr; // NUM_CASCADES
};
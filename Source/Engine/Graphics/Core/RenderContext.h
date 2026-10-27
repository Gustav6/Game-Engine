#pragma once

#include <array>

class RenderHardwareInterface;
class BuiltinAssets;
class ConstantBufferManager;
class PostProcessFilters;

struct ShadowCascade;
struct SceneCamera;
struct RenderSettings;

enum class DebugRenderMode;
enum class ToneMapper;

struct RenderContext
{
    ConstantBufferManager* ConstantBuffers = nullptr;
    RenderHardwareInterface* RHI = nullptr;
    const BuiltinAssets* BuiltinAssets = nullptr;
    const PostProcessFilters* PostProcessFilters = nullptr;
    const SceneCamera* Camera = nullptr;
    RenderSettings* RenderSettings = {};
    const std::array<ShadowCascade, 4>* CascadeShadows = nullptr; // NUM_CASCADES
};
#pragma once

#include <array>

class RenderHardwareInterface;
class BuiltinAssets;
class BufferManager;
class PostProcessFilters;
class Texture;

struct ShadowCascade;
struct CameraData;
struct RenderSettings;

enum class DebugRenderMode;
enum class ToneMapper;

struct RenderContext
{
    BufferManager* Buffers = nullptr;
    RenderHardwareInterface* RHI = nullptr;
    const BuiltinAssets* BuiltinAssets = nullptr;
    const PostProcessFilters* PostProcessFilters = nullptr;
    const CameraData* Camera = nullptr;
    RenderSettings* RenderSettings = {};
    const std::array<ShadowCascade, 4>* CascadeShadows = nullptr; // NUM_CASCADES
    std::array<const Texture*, 5> GBufferTextures {}; // NUM_GBUFFER_TARGETS
};
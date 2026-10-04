#pragma once

#include <array>

class RenderHardwareInterface;
class BuiltinAssets;
class ConstantBufferManager;

struct ShadowCascade;
struct SceneCamera;

enum class RenderMode;
enum class ToneMappingOperator;

struct RenderContext
{
    ConstantBufferManager* ConstantBuffers = nullptr;
    RenderHardwareInterface* RHI = nullptr;
    const BuiltinAssets* BuiltinAssets = nullptr;
    const SceneCamera* Camera = nullptr;
    RenderMode Mode = {};
    ToneMappingOperator Tonmap = {};
    const std::array<ShadowCascade, 4>* CascadeShadows = nullptr; // NUM_CASCADES
};
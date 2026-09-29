#pragma once

#include <vector>
#include <memory>

#include "GraphicsEngine/Core/SpriteRenderResources.h"

class GraphicsCommandList;
class RenderHardwareInterface;
class PipelineStateObject;
class Texture;

struct RenderContext;
struct FrameTargets;

class ScreenSpaceEffectPass
{
public:
    ScreenSpaceEffectPass(RenderHardwareInterface& aRHI);

    void Execute(
        GraphicsCommandList& inoutCommandList,
        FrameTargets& aFrameTargets,
        RenderContext& aContext,
        Texture* aHDRTexture
    );

private:
	std::unique_ptr<PipelineStateObject> myTonemapPSO;
};

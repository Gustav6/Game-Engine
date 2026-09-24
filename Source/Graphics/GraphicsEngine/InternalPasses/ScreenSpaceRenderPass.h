#pragma once

#include <vector>

#include "GraphicsEngine/Core/SpriteRenderResources.h"

class GraphicsCommandList;
class RenderHardwareInterface;

struct RenderContext;
struct FrameTargets;

class ScreenSpaceRenderPass
{
public:
    ScreenSpaceRenderPass(RenderHardwareInterface& aRHI);

    void Execute(
        GraphicsCommandList& inoutCommandList,
        FrameTargets& aFrameTargets,
        RenderContext& aContext
    );

private:
    SpriteRenderResources mySpriteRenderResources;
};

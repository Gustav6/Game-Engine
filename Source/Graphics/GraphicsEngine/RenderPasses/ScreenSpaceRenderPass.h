#pragma once

#include <vector>

class GraphicsCommandList;
class GraphicsEngine;

struct RenderContext;
struct FrameTargets;

class ScreenSpaceRenderPass
{
public:
    void Execute(
        GraphicsEngine& aGraphicsEngine,
        GraphicsCommandList& inoutCommandList,
        FrameTargets& aFrameTargets,
        RenderContext& aContext
    );
};

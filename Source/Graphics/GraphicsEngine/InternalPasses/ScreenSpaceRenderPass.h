#pragma once

#include <vector>

class GraphicsCommandList;

struct RenderContext;
struct FrameTargets;

class ScreenSpaceRenderPass
{
public:
    void Execute(
        GraphicsCommandList& inoutCommandList,
        FrameTargets& aFrameTargets,
        RenderContext& aContext
    );
};

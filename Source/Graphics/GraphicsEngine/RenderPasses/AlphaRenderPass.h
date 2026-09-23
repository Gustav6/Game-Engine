#pragma once

#include <vector>

class GraphicsCommandList;
class GraphicsEngine;

struct RenderItem;
struct RenderContext;
struct FrameTargets;

class AlphaRenderPass
{
public:
    bool Initialize(RenderHardwareInterface& aRHI);

    void Execute(
        GraphicsEngine& aGraphicsEngine,
        GraphicsCommandList& inoutCommandList,
        const std::vector<RenderItem>& aItems,
        FrameTargets& aFrameTargets,
        RenderContext& aContext
    );
};
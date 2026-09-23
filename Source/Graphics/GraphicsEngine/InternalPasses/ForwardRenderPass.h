#pragma once

#include <vector>

class GraphicsCommandList;

struct RenderItem;
struct RenderContext;
struct FrameTargets;

class ForwardRenderPass
{
public:
    bool Initialize(RenderHardwareInterface& aRHI);

    void Execute(
        GraphicsCommandList& inoutCommandList,
        const std::vector<RenderItem>& aItems,
        FrameTargets& aFrameTargets,
        RenderContext& aContext
    );
};
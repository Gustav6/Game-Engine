#pragma once

#include <vector>

class GraphicsCommandList;
class RenderHardwareInterface;
class PipelineStateObject;

struct RenderItem;
struct RenderContext;
struct FrameTargets;

class DebugRenderPass
{
public:
    bool Initialize(RenderHardwareInterface& aRHI);

    void Execute(
        GraphicsCommandList& inoutCommandList,
        FrameTargets& aFrameTargets,
        RenderContext& aContext
    );

private:
	std::unique_ptr<PipelineStateObject> myDebugPSO;
};
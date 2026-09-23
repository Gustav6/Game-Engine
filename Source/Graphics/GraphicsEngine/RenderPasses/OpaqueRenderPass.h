#pragma once

#include <vector>
#include <filesystem>

class GraphicsCommandList;
class GraphicsEngine;
class RenderHardwareInterface;
class GBuffer;
class PipelineStateObject;

struct RenderItem;
struct RenderContext;
struct FrameTargets;

class OpaqueRenderPass
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

private:
    std::unique_ptr<PipelineStateObject> myGBufferPSO;
    std::unique_ptr<PipelineStateObject> myDeferredAmbientPSO;
    std::unique_ptr<PipelineStateObject> myDeferredDirPSO;
    std::unique_ptr<PipelineStateObject> myDeferredSpotPSO;
    std::unique_ptr<PipelineStateObject> myDeferredPointPSO;

    std::unique_ptr<GBuffer> myGBuffer;
};


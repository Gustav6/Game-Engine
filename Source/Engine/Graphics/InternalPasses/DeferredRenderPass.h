#pragma once

#include <vector>

class GraphicsCommandList;
class RenderHardwareInterface;
class GBuffer;
class PipelineStateObject;
class Texture;

struct RenderItem;
struct RenderContext;
struct FrameTargets;

class DeferredRenderPass
{
public:
    bool Initialize(RenderHardwareInterface& aRHI);

    void Execute(
        GraphicsCommandList& inoutCommandList,
        const std::vector<RenderItem>& aItems,
        FrameTargets& aFrameTargets,
        RenderContext& aContext
    );

    void SetSSAOStatus(bool aEnabled);

private:
    std::unique_ptr<PipelineStateObject> myDeferredAmbientPSO;
    std::unique_ptr<PipelineStateObject> myDeferredDirPSO;
    std::unique_ptr<PipelineStateObject> myDeferredSpotPSO;
    std::unique_ptr<PipelineStateObject> myDeferredPointPSO;
    std::unique_ptr<PipelineStateObject> mySSAOPSO;

    std::unique_ptr<GBuffer> myGBuffer;

    std::vector<const Texture*> myGBufferTextures;

    std::unique_ptr<Texture> myHalfSizeSSAOTexture;
    std::unique_ptr<Texture> myFullSizeSSAOTexture;
    std::unique_ptr<Texture> myBlurSSAOTexture;

    bool mySSAOEnabled;
};


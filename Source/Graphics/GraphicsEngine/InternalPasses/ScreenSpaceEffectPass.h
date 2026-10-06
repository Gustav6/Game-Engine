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

    void ToggleBloom();

private:
	void SSAOPass();

    void BloomPass(
        GraphicsCommandList& inoutCommandList,
        RenderContext& aContext,
        Texture* aHDRTexture);
    void TonemapPass(
        GraphicsCommandList& inoutCommandList,
        RenderContext& aContext,
        Texture* aHDRTexture);

    std::unique_ptr<PipelineStateObject> mySSAOPSO;
    std::unique_ptr<PipelineStateObject> myTonemapPSO;
    std::unique_ptr<PipelineStateObject> myLuminancePSO;
    std::unique_ptr<PipelineStateObject> myResamplePSO;
    std::unique_ptr<PipelineStateObject> myGaussianBlurVerticalPSO;
    std::unique_ptr<PipelineStateObject> myGaussianBlurHorizontalPSO;
    std::unique_ptr<PipelineStateObject> myBloomPSO;

    std::unique_ptr<Texture> mySceneLuminance;
    std::unique_ptr<Texture> myResampleFullSize;
    std::unique_ptr<Texture> myResampleHalfSize;
    std::unique_ptr<Texture> myResampleQuarterSize1;
    std::unique_ptr<Texture> myResampleQuarterSize2;
    std::unique_ptr<Texture> myBlurredTexture;
    std::unique_ptr<Texture> myBloomTexture;

    bool myBloomEnabled;
};

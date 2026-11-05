#pragma once

#include <memory>

class GraphicsCommandList;
class RenderHardwareInterface;
class PipelineStateObject;
class Texture;

class PostProcessFilters
{
public:
    bool Initialize(RenderHardwareInterface& aRHI);

    void BloomDownsample(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;
    void BloomAdditiveUpsample(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;

    void Resample(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;

    void BlurHorizontal(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;
    void BlurVertical(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;

    void BlurDepthSSAO(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination, const Texture& aNormalDepth) const;

private:
	std::unique_ptr<PipelineStateObject> myDownsamplePSO;
	std::unique_ptr<PipelineStateObject> myAdditiveUpsamplePSO;
    std::unique_ptr<PipelineStateObject> myBlurHorizontalPSO;
    std::unique_ptr<PipelineStateObject> myBlurVerticalPSO;
    std::unique_ptr<PipelineStateObject> myDepthBlurSSAOPSO;
	std::unique_ptr<PipelineStateObject> myResamplePSO;
};
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

    void Downsample( GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;
    void AdditiveUpsample(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;

    void BlurHorizontal(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;
    void BlurVertical(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;

private:
	std::unique_ptr<PipelineStateObject> myDownsamplePSO;
	std::unique_ptr<PipelineStateObject> myAdditiveUpsamplePSO;
    std::unique_ptr<PipelineStateObject> myBlurHorizontalPSO;
    std::unique_ptr<PipelineStateObject> myBlurVerticalPSO;
};
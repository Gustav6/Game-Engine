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

    void Resample(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;

    void BlurHorizontal(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;
    void BlurVertical(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const;

private:
    std::unique_ptr<PipelineStateObject> myResamplePSO;
    std::unique_ptr<PipelineStateObject> myBlurHorizontalPSO;
    std::unique_ptr<PipelineStateObject> myBlurVerticalPSO;
};
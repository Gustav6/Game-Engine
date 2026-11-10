#pragma once

#include <memory>
#include <vector>

class GraphicsCommandList;
class RenderHardwareInterface;
class PipelineStateObject;
class Texture;

struct RenderItem;
struct RenderContext;

class PixelPickingPass
{
public:
    PixelPickingPass() = default;
    ~PixelPickingPass();

    bool Initialize(RenderHardwareInterface& aRHI);

    void Execute(GraphicsCommandList& inoutCommandList, const std::vector<RenderItem>& aItems, Texture& aDepthBuffer, RenderContext& aContext);

    const Texture* GetIDBuffer() const;
    const Texture* GetIDStagingBuffer() const;

private:
    std::unique_ptr<PipelineStateObject> myPickingPSO;

    std::unique_ptr<Texture> myIDBuffer;
    std::unique_ptr<Texture> myIDStagingBuffer;
};
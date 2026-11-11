#pragma once

#include <memory>
#include <vector>

#include "CommonUtilities/Math/Vector2.hpp"

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

    void Execute(GraphicsCommandList& inoutCommandList, const std::vector<RenderItem>& aItems, RenderContext& aContext);

    void RequestPixelPick(unsigned aX, unsigned aY);
    bool HasPendingPick() const;

    std::optional<unsigned> TryGetPickResult(const RenderHardwareInterface& aRHI);

private:
    std::unique_ptr<PipelineStateObject> myPickingPSO;

    std::unique_ptr<Texture> myIDBuffer;
    std::unique_ptr<Texture> myIDStagingBuffer;
    std::unique_ptr<Texture> myPickDepth;

    std::optional<CommonUtilities::Vector2ui> myPendingPick;
    bool myHasAPickResult = false;
};
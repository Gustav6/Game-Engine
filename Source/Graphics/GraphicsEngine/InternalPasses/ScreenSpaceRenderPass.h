#pragma once

#include <vector>

#include "GraphicsEngine/Core/SpriteRenderResources.h"

class GraphicsCommandList;
class RenderHardwareInterface;

struct RenderContext;
struct FrameTargets;
struct ScreenSpaceItem;

struct TextDrawCall;
struct SpriteDrawCall;

class ScreenSpaceRenderPass
{
public:
    ScreenSpaceRenderPass(RenderHardwareInterface& aRHI);

    void Execute(
        GraphicsCommandList& inoutCommandList,
        const std::vector<ScreenSpaceItem>& aItems,
        FrameTargets& aFrameTargets,
        RenderContext& aContext
    );

private:
    void RenderText(GraphicsCommandList& inoutCommandList, const TextDrawCall& aDrawCall, RenderContext& aContext);
    void RenderSprite(GraphicsCommandList& inoutCommandList, const SpriteDrawCall& aDrawCall, RenderContext& aContext);

    SpriteRenderResources mySpriteRenderResources;
};

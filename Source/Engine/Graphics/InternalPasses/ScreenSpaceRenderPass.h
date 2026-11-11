#pragma once

#include <vector>

#include "CommonUtilities/Math/Matrix4x4.hpp"

#include "Graphics/Objects/GPU/Buffer.h"
#include "Graphics/Objects/GPU/Vertex.h"

class GraphicsCommandList;
class RenderHardwareInterface;
class PipelineStateObject;

struct RenderContext;
struct FrameTargets;
struct ScreenSpaceItem;

struct TextDrawCall;
struct SpriteDrawCall;

class ScreenSpaceRenderPass
{
public:
	bool Initialize(RenderHardwareInterface& aRHI);

    void Execute(
        GraphicsCommandList& inoutCommandList,
        const std::vector<ScreenSpaceItem>& aItems,
        FrameTargets& aFrameTargets,
        RenderContext& aContext
    );

private:
    void RenderText(GraphicsCommandList& inoutCommandList, const TextDrawCall& aDrawCall, RenderContext& aContext);
    void RenderSprite(GraphicsCommandList& inoutCommandList, const SpriteDrawCall& aDrawCall, RenderContext& aContext);
    
    std::unique_ptr<PipelineStateObject> myFontPSO;
    std::unique_ptr<PipelineStateObject> mySpritePSO;

    CommonUtilities::Matrix4x4f mySpriteProjection;
    std::vector<SpriteVertex> mySpriteVertices;
    Buffer mySpriteVertexBuffer;
};

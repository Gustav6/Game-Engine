#pragma once

#include <vector>


#include "Graphics/Objects/GPU/Vertex.h"

class GraphicsCommandList;
class RenderHardwareInterface;
class PipelineStateObject;
class Buffer;
class Texture;

struct RenderContext;
struct FrameTargets;

class DebugRenderPass
{
public:
    bool Initialize(RenderHardwareInterface& aRHI);

    void Execute(
        GraphicsCommandList& inoutCommandList,
        FrameTargets& aFrameTargets,
        RenderContext& aContext
    );

private:
	void RenderWireMeshes(GraphicsCommandList& inoutCommandList, RenderContext& aContext);
    void RenderLines(GraphicsCommandList& inoutCommandList, RenderContext& aContext);
    void RenderOutlines(GraphicsCommandList& inoutCommandList, RenderContext& aContext, FrameTargets& aFrameTargets);

	std::unique_ptr<PipelineStateObject> myMeshWireframePSO;
    std::unique_ptr<PipelineStateObject> myMeshOutlinePSO;
    std::unique_ptr<PipelineStateObject> myMeshOutlineMaskPSO;
	std::unique_ptr<PipelineStateObject> myLinePSO;

    std::vector<LineVertex> myLineVertices;
    std::unique_ptr<Buffer> myLineVertexBuffer;
    std::unique_ptr<Buffer> myInstanceBuffer;

    std::unique_ptr<Texture> myOutlineBuffer;
};
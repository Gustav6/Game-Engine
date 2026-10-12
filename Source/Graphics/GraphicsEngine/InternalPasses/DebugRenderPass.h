#pragma once

#include <vector>

#include "Objects/GPU/Buffer.h"
#include "Objects/GPU/Vertex.h"

class GraphicsCommandList;
class RenderHardwareInterface;
class PipelineStateObject;

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

	std::unique_ptr<PipelineStateObject> myDebugPSO;
	std::unique_ptr<PipelineStateObject> myLinePSO;

    std::vector<Vertex> myLineVertices;
    Buffer myLineVertexBuffer;
};
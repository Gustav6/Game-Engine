#pragma once

#include <vector>

class GraphicsCommandList;
class Mesh;

struct RenderItem;
struct RenderContext;

enum DrawCallFlag_
{
	DrawCallFlag_None = 0,
	DrawCallFlag_BindTextures = 1 << 0,
	DrawCallFlag_BindMaterial = 1 << 1
};

typedef int DrawCallFlags;

class DrawCallSubmitter
{
public:
    static void RenderDrawCalls(
        GraphicsCommandList& inoutCommandList,
        const std::vector<RenderItem>& aItems,
        RenderContext& aContext,
        DrawCallFlags someFlags
    );

private:
    static bool PrepareMeshForRendering(const Mesh& aMesh, RenderHardwareInterface& aRHI);
    static bool HasDrawCallFlag(DrawCallFlags aFlags, DrawCallFlag_ aFlag);
};
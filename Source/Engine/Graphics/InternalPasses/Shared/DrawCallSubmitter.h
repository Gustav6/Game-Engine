#pragma once

#include <vector>
#include <memory>

class RenderHardwareInterface;
class GraphicsCommandList;
class Mesh;
class Texture;
class Buffer;

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
    static DrawCallSubmitter& Get();

	bool Initialize(RenderHardwareInterface& aRHI);

    void RenderDrawCalls(
        GraphicsCommandList& inoutCommandList,
        const std::vector<RenderItem>& aItems,
        RenderContext& aContext,
        DrawCallFlags someFlags
    );

private:
    DrawCallSubmitter();

    bool PrepareMeshForRendering(const Mesh& aMesh, RenderHardwareInterface& aRHI);
    bool HasDrawCallFlag(DrawCallFlags aFlags, DrawCallFlag_ aFlag);

	std::unique_ptr<Buffer> myInstanceBuffer;
};
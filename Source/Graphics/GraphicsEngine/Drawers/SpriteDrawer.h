#pragma once

#include <vector>

struct SpriteBatch;

struct SpriteInstance;
struct SpriteShared;

struct SpriteDrawCall
{
	const SpriteInstance* SpriteInstance = nullptr;
	const SpriteShared* SpriteShared = nullptr;
};

class SpriteDrawer
{
	friend class GraphicsEngine;
	friend class ScreenSpaceRenderPass;

public:
	static SpriteDrawer& Get();

	void Submit(const SpriteDrawCall& aDrawCall);

private:
	std::vector<SpriteDrawCall> myDrawCalls;
};
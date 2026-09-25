#pragma once

#include <vector>

class MaterialInterface;

struct SpriteBatch;
struct SpriteInstanceData;
struct SpriteSharedData;

struct SpriteDrawCall
{
	const SpriteInstanceData* SpriteInstanceData = nullptr;
	const SpriteSharedData* SpriteSharedData = nullptr;
	const MaterialInterface* Material;
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
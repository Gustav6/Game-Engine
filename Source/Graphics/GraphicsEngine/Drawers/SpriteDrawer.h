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
public:
	static SpriteDrawer& Get();

	void Submit(const SpriteDrawCall& aDrawCall);

	const std::vector<SpriteDrawCall>& GetSpriteDrawCalls() const;

	void Clear();

private:
	std::vector<SpriteDrawCall> myDrawCalls;
};
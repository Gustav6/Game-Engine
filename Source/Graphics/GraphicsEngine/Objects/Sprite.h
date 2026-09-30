#pragma once

#include <string>
#include "GPU/Texture.h"

#include "CommonUtilities/Math/Vector2.hpp"
#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Math/Vector4.hpp"

struct SpriteSharedData
{
	std::string Name;
	Texture Texture;
};

struct SpriteInstanceData
{
	CommonUtilities::Vector2f Size = { 1.0f, 1.0f };
	CommonUtilities::Vector2f Pivot = { 0.5f, 0.5f };
	CommonUtilities::Vector2f Anchor = { 0.0f, 0.0f };
	CommonUtilities::Vector2f Position = { 0.0f, 0.0f };

	CommonUtilities::Vector4f Color = { 1, 1, 1, 1 };

	float Depth = 0;

	bool IsVisible = true;
};

//struct SpriteAtlasRegion
//{
//	CommonUtilities::Vector2f UVMin = { 0.0f, 0.0f };
//	CommonUtilities::Vector2f UVMax = { 1.0f, 1.0f };
//};
//
//inline SpriteAtlasRegion MakeAtlasRegion(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
//{
//	constexpr float atlasWidth = 256.0f;
//	constexpr float atlasHeight = 256.0f;
//
//	SpriteAtlasRegion region;
//
//	region.UVMin = { x / atlasWidth, y / atlasHeight };
//
//	region.UVMax = { (x + width) / atlasWidth, (y + height) / atlasHeight };
//
//	return region;
//}

//
//struct SpriteBatch
//{
//	Texture* Texture = nullptr;
//	size_t StartVertex = 0;
//	size_t VertexCount = 0;
//};

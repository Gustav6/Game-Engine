#pragma once

#include <memory>

#include <CommonUtilities/Math/Vector.hpp>

class Texture;

struct SpriteAtlasRegion
{
	CommonUtilities::Vector2f UVMin = { 0.0f, 0.0f };
	CommonUtilities::Vector2f UVMax = { 1.0f, 1.0f };
};

inline SpriteAtlasRegion MakeAtlasRegion(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	constexpr float atlasWidth = 256.0f;
	constexpr float atlasHeight = 256.0f;

	SpriteAtlasRegion region;

	region.UVMin = { x / atlasWidth, y / atlasHeight };

	region.UVMax = { (x + width) / atlasWidth, (y + height) / atlasHeight };

	return region;
}

struct SpriteShared
{
	//SpriteAtlasRegion AtlasRegion;
	
	// TODO: Add name here

	std::shared_ptr<Texture> TextureResource = nullptr;
};

struct SpriteInstance 
{
	CommonUtilities::Vector2f Size = { 1.0f, 1.0f };
	CommonUtilities::Vector2f Pivot = { 0.5f, 0.5f };
	CommonUtilities::Vector2f Anchor = { 0.0f, 0.0f };
	CommonUtilities::Vector2f Position = { 0.0f, 0.0f };

	CommonUtilities::Vector4f Color = { 1, 1, 1, 1 };

	float Z = 0.0f;
	bool IsVisible = true;
};

struct SpriteBatch
{
	Texture* Texture = nullptr;
	size_t StartVertex = 0;
	size_t VertexCount = 0;
};

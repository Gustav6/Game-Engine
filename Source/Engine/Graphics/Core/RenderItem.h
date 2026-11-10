#pragma once

struct MeshDrawCall;
struct TextDrawCall;
struct SpriteDrawCall;

struct RenderItem
{
    const MeshDrawCall* DrawCall;
    unsigned int ElementIndex;
    unsigned int MaterialIndex;
    unsigned int ObjectID;
};

struct ScreenSpaceItem
{
    const TextDrawCall* TextCall = nullptr;
    const SpriteDrawCall* SpriteCall = nullptr;

    float Depth = 0.0f;
};
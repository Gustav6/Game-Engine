#pragma once

struct MeshDrawCall;
struct TextDrawCall;
struct SpriteDrawCall;

struct RenderItem
{
    const MeshDrawCall* DrawCall;
    unsigned int ElementIndex;
    unsigned int MaterialIndex;
};

struct ScreenSpaceItem
{
    enum class Kind 
    { 
        Text, 
        Sprite 
    };

    Kind ItemKind;

    const TextDrawCall* TextCall = nullptr;
    const SpriteDrawCall* SpriteCall = nullptr;

    float SortOrder = 0.0f;
};
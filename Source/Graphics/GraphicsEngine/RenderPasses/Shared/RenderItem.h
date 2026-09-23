#pragma once

struct MeshDrawCall;

struct RenderItem
{
    const MeshDrawCall* DrawCall;
    unsigned int ElementIndex;
    unsigned int MaterialIndex;
};
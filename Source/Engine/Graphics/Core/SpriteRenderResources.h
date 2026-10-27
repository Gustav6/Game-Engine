#pragma once

#include <vector>

#include <CommonUtilities/Math/Matrix4x4.hpp>

#include "Graphics/Objects/GPU/Vertex.h"
#include "Graphics/Objects/GPU/Buffer.h"

struct SpriteRenderResources
{
    CommonUtilities::Matrix4x4f Projection;
    std::vector<Vertex> Vertices;
    Buffer VertexBuffer;
};
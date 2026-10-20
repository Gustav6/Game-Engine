#pragma once

#include "CommonUtilities/Math/Vector2.hpp"
#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Math/Vector4.hpp"
#include "GraphicsEngine/RHI/RHIStructs.h"

struct Vertex
{
	CommonUtilities::Vector4f Position = { 0, 0, 0, 1 };
	CommonUtilities::Vector4f Color = { 1, 1, 1, 1 };
	CommonUtilities::Vector4ui BoneIDs = { 0, 0, 0, 0 };
	CommonUtilities::Vector4f SkinWeights = { 0, 0, 0, 0 };

	CommonUtilities::Vector2f UV0 = { 0, 0 };
	CommonUtilities::Vector2f UV1 = { 0, 0 };

	CommonUtilities::Vector3f Normal = { 0, 0, 0 };
	CommonUtilities::Vector3f Tangent = { 0, 0, 0 };

	static const std::vector<VertexElementDescription> Description;
};

struct LineVertex
{
	CommonUtilities::Vector4f Position = { 0, 0, 0, 1 };
	CommonUtilities::Vector4f Color = { 1, 1, 1, 1 };

	static const std::vector<VertexElementDescription> Description;
};

struct ParticleVertex
{
	CommonUtilities::Vector4f Position;
	CommonUtilities::Vector4f Color;
	CommonUtilities::Vector3f Velocity;
	float Lifetime;
	float Angle;
	CommonUtilities::Vector2f Size;
	CommonUtilities::Vector4f ChannelMask;

	static const std::vector<VertexElementDescription> Description;
};

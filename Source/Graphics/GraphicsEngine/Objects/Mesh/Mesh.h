#pragma once

#include <memory>
#include <vector>

#include "CommonUtilities/Collision/Sphere.hpp"
#include "CommonUtilities/Collision/AABB3D.hpp"

#include "GraphicsEngine/Objects/GPU/Buffer.h"

struct Skeleton;
struct Vertex;

class Mesh
{
	friend class GraphicsEngine;
	friend class DrawCallSubmitter;

public:
	struct Element
	{
		// The start position in the vertex array / buffer
		unsigned int VertexOffset = 0;

		// The start position in the index array / buffer
		unsigned int IndexOffset = 0;

		// How many vertices this element is
		unsigned int NumVertices = 0;

		// How many indices this element is
		unsigned int NumIndices = 0;

		// The material index of this element
		unsigned int MaterialIndex = 0;

		CommonUtilities::Sphere<float> BoundingSphere;
		CommonUtilities::AABB3D<float> BoundingBox;
	};

	Mesh();
	~Mesh();

	Mesh& operator=(Mesh&) = delete;
	Mesh& operator=(Mesh&& aOther) noexcept = delete;

	void Initialize(
		std::string aName, 
		std::vector<Element>&& aElementList, 
		std::vector<Vertex>&& aVertexList, 
		std::vector<unsigned>&& aIndexList,
		std::unique_ptr<Skeleton> aSkeleton,
		CommonUtilities::Sphere<float>& aBoundingSphere,
		CommonUtilities::AABB3D<float>& aBoundingBox);

	std::string_view GetName() const;
	Skeleton* GetSkeleton() const;
	size_t GetNumMaterialSlots() const;
	const CommonUtilities::Sphere<float>& GetBoundingSphere() const;
	const CommonUtilities::AABB3D<float>& GetBoundingBox() const;

private:
	std::string myName;
	std::vector<Vertex> myVertices;
	std::vector<unsigned int> myIndices;
	std::vector<Element> myElements;

	size_t myNumMaterialSlots = 0;

	mutable Buffer myVertexBuffer;
	mutable Buffer myIndexBuffer;

	std::unique_ptr<Skeleton> mySkeleton;

	CommonUtilities::Sphere<float> myBoundingSphere;
	CommonUtilities::AABB3D<float> myBoundingBox;
};
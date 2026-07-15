#pragma once

#include <memory>
#include <vector>

#include "CommonUtilities/Sphere.hpp"

#include "Buffer.h"

struct Skeleton;
struct Vertex;

class Mesh
{
	friend class GraphicsEngine;

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
	};

	Mesh();
	~Mesh();

	Mesh& operator=(Mesh&) = delete;
	Mesh& operator=(Mesh&& aOther) noexcept = delete;

	void Initialize(std::string aName, 
					std::vector<Element>&& aElementList, 
					std::vector<Vertex>&& aVertexList, 
					std::vector<unsigned>&& aIndexList,
					std::unique_ptr<Skeleton> aSkeleton,
					std::unique_ptr<CommonUtilities::Sphere<float>> aBoundingSphere);

	std::string_view GetName() const;
	Skeleton* GetSkeleton() const;
	size_t GetNumMaterialSlots() const;
	CommonUtilities::Sphere<float>* GetBoundingSphere() const;

private:
	std::string myName;
	std::vector<Vertex> myVertices;
	std::vector<unsigned int> myIndices;
	std::vector<Element> myElements;

	size_t myNumMaterialSlots = 0;

	mutable Buffer myVertexBuffer;
	mutable Buffer myIndexBuffer;

	std::unique_ptr<Skeleton> mySkeleton;
	std::unique_ptr<CommonUtilities::Sphere<float>> myBoundingSphere;
};
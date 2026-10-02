#include "GraphicsEngine.pch.h"
#include "Mesh.h"

#include "GraphicsEngine/Objects/GPU/Vertex.h"
#include "Skeleton.h"

Mesh::Mesh() = default;
Mesh::~Mesh() = default;

void Mesh::Initialize(
	std::string aName, 
	std::vector<Element>&& aElementList, 
	std::vector<Vertex>&& aVertexList, 
	std::vector<unsigned>&& aIndexList, 
	std::unique_ptr<Skeleton> aSkeleton,
	CommonUtilities::Sphere<float>& aBoundingSphere,
	CommonUtilities::AABB3D<float>& aBoundingBox)
{
	myName = aName;
	myElements = std::move(aElementList);
	myVertices = std::move(aVertexList);
	myIndices = std::move(aIndexList);
	mySkeleton = std::move(aSkeleton);
	myBoundingSphere = aBoundingSphere;
	myBoundingBox = aBoundingBox;

	std::unordered_set<unsigned int> materialIds;
	for (const Element& element : myElements)
	{
		materialIds.emplace(element.MaterialIndex);
	}

	myNumMaterialSlots = materialIds.size();
}

std::string_view Mesh::GetName() const
{
	return myName;
}

Skeleton* Mesh::GetSkeleton() const
{
	return mySkeleton.get();
}

size_t Mesh::GetNumMaterialSlots() const
{
	return myNumMaterialSlots;
}

const CommonUtilities::Sphere<float>& Mesh::GetBoundingSphere() const
{
	return myBoundingSphere;
}

const CommonUtilities::AABB3D<float>& Mesh::GetBoundingBox() const
{
	return myBoundingBox;
}

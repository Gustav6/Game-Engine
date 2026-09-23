#include "GraphicsEngine.pch.h"
#include "Mesh.h"

#include "../GPU/Vertex.h"
#include "Skeleton.h"

Mesh::Mesh() = default;
Mesh::~Mesh() = default;

void Mesh::Initialize(std::string aName, 
					  std::vector<Element>&& aElementList, 
					  std::vector<Vertex>&& aVertexList, 
					  std::vector<unsigned>&& aIndexList, 
					  std::unique_ptr<Skeleton> aSkeleton,
					  std::unique_ptr<CommonUtilities::Sphere<float>> aBoundingSphere)
{
	myName = aName;
	myElements = std::move(aElementList);
	myVertices = std::move(aVertexList);
	myIndices = std::move(aIndexList);
	mySkeleton = std::move(aSkeleton);
	myBoundingSphere = std::move(aBoundingSphere);

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

CommonUtilities::Sphere<float>* Mesh::GetBoundingSphere() const
{
	return myBoundingSphere.get();
}

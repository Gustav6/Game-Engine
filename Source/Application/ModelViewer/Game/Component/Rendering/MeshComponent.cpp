#include "MeshComponent.h"

#include "Ensure.h"

#include "GraphicsEngine/Objects/Mesh.h"
#include "Game/GameObject.hpp"

MeshComponent::MeshComponent(GameObject* aOwner)
	: Component(aOwner)
{}

std::shared_ptr<Mesh> MeshComponent::GetMesh()
{
	return myMesh;
}

void MeshComponent::SetMesh(const std::shared_ptr<Mesh>& aMesh)
{
	myMesh = aMesh;
	myMaterials.resize(myMesh->GetNumMaterialSlots());
}

Skeleton* MeshComponent::GetSkeleton() const
{
	return myMesh->GetSkeleton();
}

void MeshComponent::SetMaterial(unsigned int aMaterialIndex, const std::shared_ptr<MaterialInterface>& aMaterial)
{
	ensure(aMaterialIndex < myMaterials.size());

	myMaterials[aMaterialIndex] = aMaterial;
}

const std::vector<std::shared_ptr<MaterialInterface>>& MeshComponent::GetMaterialList() const
{
	return myMaterials;
}

CommonUtilities::Sphere<float> MeshComponent::GetWorldSpaceBoundingSphere() const
{
	const CommonUtilities::Vector3f worldPosition = myOwner->GetTransform().GetPosition();
	const CommonUtilities::Vector3f& meshCenter = myMesh->GetBoundingSphere()->GetCenter();
	const float meshRadius = myMesh->GetBoundingSphere()->GetRadius();

	return CommonUtilities::Sphere<float>(meshCenter + worldPosition, meshRadius);
}

CommonUtilities::Sphere<float> MeshComponent::GetLocalSpaceBoundingSphere() const
{
	return *myMesh->GetBoundingSphere();
}

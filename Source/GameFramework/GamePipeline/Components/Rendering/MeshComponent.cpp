#include "GameFramework.pch.h"
#include "MeshComponent.h"

#include "Ensure.h"

#include <AssetPipeline/Assets/Types/MeshAsset.h>
#include <AssetPipeline/Assets/Types/MaterialAsset.h>

#include "GamePipeline/World/Actor.hpp"

#include "GraphicsEngine/Objects/Mesh.h"

MeshComponent::MeshComponent(Actor* aOwner)
	: Component(aOwner)
{}

void MeshComponent::SetMesh(const std::shared_ptr<MeshAsset>& aMeshAsset)
{
	myMeshAsset = aMeshAsset;
	myMaterialAssets.resize(myMeshAsset->GetMesh().GetNumMaterialSlots());
}

const Mesh& MeshComponent::GetMesh()
{
	return myMeshAsset->GetMesh();
}

Skeleton* MeshComponent::GetSkeleton() const
{
	return myMeshAsset->GetMesh().GetSkeleton();
}

void MeshComponent::SetMaterial(unsigned int aMaterialIndex, std::shared_ptr<MaterialAsset> aMaterialAsset)
{
	ensure(aMaterialIndex < myMaterialAssets.size());

	myMaterialAssets[aMaterialIndex] = aMaterialAsset;
}

const std::vector<std::shared_ptr<MaterialAsset>>& MeshComponent::GetMaterialAssetList() const
{
	return myMaterialAssets;
}

CommonUtilities::Sphere<float> MeshComponent::GetWorldSpaceBoundingSphere() const
{
	const CommonUtilities::Vector3f worldPosition = myOwner->GetTransform().GetPosition();
	const CommonUtilities::Vector3f& meshCenter = myMeshAsset->GetMesh().GetBoundingSphere()->GetCenter();
	const float meshRadius = myMeshAsset->GetMesh().GetBoundingSphere()->GetRadius();

	return CommonUtilities::Sphere<float>(meshCenter + worldPosition, meshRadius);
}

CommonUtilities::Sphere<float> MeshComponent::GetLocalSpaceBoundingSphere() const
{
	return *myMeshAsset->GetMesh().GetBoundingSphere();
}

#include "GameFramework.pch.h"
#include "MeshComponent.h"

#include "AnimationComponent.h"

#include <AssetPipeline/Assets/Types/MeshAsset.h>
#include <AssetPipeline/Assets/Types/MaterialAsset.h>

#include "GamePipeline/World/Actor.hpp"

#include "GraphicsEngine/Objects/Mesh/Mesh.h"

#include <GraphicsEngine/Drawers/MeshDrawer.h>

#include "Ensure.h"

MeshComponent::MeshComponent(Actor* aOwner)
	: Component(aOwner)
{}

void MeshComponent::Render()
{
	MeshDrawCall drawCall =
	{
		.Mesh = &GetMesh(),
		.Transform = myOwner->GetTransform().GetWorldMatrix()
	};

	const auto& materialAssets = GetMaterialAssetList();

	for (size_t i = 0; i < materialAssets.size(); i++)
	{
		if (materialAssets[i] != nullptr)
		{
			drawCall.Materials[i] = materialAssets[i]->GetMaterial().get();
		}
	}

	AnimationComponent* animationComponent = myOwner->GetComponent<AnimationComponent>();

	if (animationComponent != nullptr)
	{
		drawCall.HasSkinning = true;
		drawCall.JointTransforms = animationComponent->GetJointTransforms();
	}

	MeshDrawer::Get().Submit(drawCall);
}

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
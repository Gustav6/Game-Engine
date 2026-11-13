#include "Core/Core.pch.h"
#include "InstancedMeshComponent.h"

#include "CommonUtilities/Math/Matrix4x4.hpp"

#include "Core/Assets/Concrete/MeshAsset.h"
#include "Core/Assets/Concrete/MaterialAsset.h"

#include "Core/Scene/World/Actor.hpp"

#include "Graphics/Objects/Mesh/Mesh.h"

#include "Graphics/Drawers/MeshDrawer.h"

InstancedMeshComponent::InstancedMeshComponent(Actor* aOwner)
	: Component(aOwner)
{}

void InstancedMeshComponent::Render()
{
    if (myMeshAsset == nullptr || myInstances.empty())
    {
        return;
    }

    MeshDrawCall drawCall =
    {
        .Mesh = &myMeshAsset->GetMesh(),
        .Transform = myOwner->GetTransform().GetWorldMatrix(),
        .ObjectID = myOwner->GetID(),
        .Instances = myInstances
    };

    for (size_t i = 0; i < myMaterialAssets.size(); i++)
    {
        if (myMaterialAssets[i] != nullptr)
        {
            drawCall.Materials[i] = myMaterialAssets[i]->GetMaterial().get();
        }
    }

    MeshDrawer::Get().Submit(drawCall);
}

void InstancedMeshComponent::AddInstance(const InstanceData& aInstanceData)
{
	myInstances.push_back(aInstanceData);
}

void InstancedMeshComponent::RemoveInstance(unsigned int aIndex)
{
	ensure(aIndex < myInstances.size());
	myInstances.erase(myInstances.begin() + aIndex);
}

void InstancedMeshComponent::ClearInstances()
{
	myInstances.clear();
}

void InstancedMeshComponent::SetMesh(const std::shared_ptr<MeshAsset>& aMeshAsset)
{
    myMeshAsset = aMeshAsset;
    myMaterialAssets.resize(myMeshAsset->GetMesh().GetNumMaterialSlots());
}

void InstancedMeshComponent::SetMaterial(unsigned int aMaterialIndex, const std::shared_ptr<MaterialAsset>& aMaterialAsset)
{
    ensure(aMaterialIndex < myMaterialAssets.size());

    myMaterialAssets[aMaterialIndex] = aMaterialAsset;
}

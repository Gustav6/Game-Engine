#include "Core/Core.pch.h"
#include "InstancedMeshComponent.h"

#include "CommonUtilities/Math/Matrix4x4.hpp"

#include "Core/Assets/Concrete/MeshAsset.h"
#include "Core/Assets/Concrete/MaterialAsset.h"
#include <Core/Scene/Components/Concrete/Transform/Transform.h>

#include "Core/Scene/World/Actor.hpp"

#include "Graphics/Objects/Mesh/Mesh.h"

#include "Graphics/Drawers/MeshDrawer.h"
#include "Graphics/Drawers/DebugDrawer.h"

InstancedMeshComponent::InstancedMeshComponent(Actor* aOwner)
	: IComponent(aOwner)
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
        .Transform = myOwner->GetComponent<Transform>()->GetWorldMatrix(),
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

void InstancedMeshComponent::RenderOutline()
{
    if (myMeshAsset == nullptr)
    {
        return;
    }

    DebugDrawer::Get().DrawMeshOutline(
        myOwner->GetComponent<Transform>()->GetWorldMatrix(),
        &myMeshAsset->GetMesh(),
        { 0.0f, 1.0f, 0.0f, 1.0f },
        myInstances.data(),
        static_cast<unsigned int>(myInstances.size())
    );
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

    if (myMeshAsset == nullptr)
    {
        return;
    }

    myMaterialAssets.resize(myMeshAsset->GetMesh().GetNumMaterialSlots());
}

void InstancedMeshComponent::SetMaterial(unsigned int aMaterialIndex, const std::shared_ptr<MaterialAsset>& aMaterialAsset)
{
    ensure(aMaterialIndex < myMaterialAssets.size());

    myMaterialAssets[aMaterialIndex] = aMaterialAsset;
}

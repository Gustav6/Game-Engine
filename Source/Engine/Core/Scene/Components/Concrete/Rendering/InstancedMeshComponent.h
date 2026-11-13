#pragma once

#include <memory>
#include <vector>

#include "Core/Scene/Components/Interface/IComponent.h"

#include "Graphics/Objects/GPU/InstanceData.h"

class MeshAsset;
class MaterialAsset;
struct Skeleton;
class Mesh;

class InstancedMeshComponent : public Component
{
public:
    InstancedMeshComponent(Actor* aOwner);

    void Render() override;

    void AddInstance(const InstanceData& aInstanceData);
    void RemoveInstance(unsigned int aIndex);
    void ClearInstances();

    void SetMesh(const std::shared_ptr<MeshAsset>& aMeshAsset);

    void SetMaterial(unsigned int aMaterialIndex, const std::shared_ptr<MaterialAsset>& aMaterialAsset);

private:
    std::shared_ptr<MeshAsset> myMeshAsset;
    std::vector<std::shared_ptr<MaterialAsset>> myMaterialAssets;

    std::vector<InstanceData> myInstances;
};
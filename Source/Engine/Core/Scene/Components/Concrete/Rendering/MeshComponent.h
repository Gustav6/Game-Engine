#pragma once

#include <memory>
#include <vector>

#include "Core/Scene/Components/Interface/Component.h"

class MeshAsset;
class MaterialAsset;
struct Skeleton;
class Mesh;

class MeshComponent : public Component
{
public:
	MeshComponent(Actor* aOwner);

	void Render() override;

	Skeleton* GetSkeleton() const;

	void SetMesh(const std::shared_ptr<MeshAsset>& aMeshAsset);
	void SetMaterial(unsigned int aMaterialIndex, const std::shared_ptr<MaterialAsset>& aMaterialAsset);

private:
	std::shared_ptr<MeshAsset> myMeshAsset;
	std::vector<std::shared_ptr<MaterialAsset>> myMaterialAssets;
};


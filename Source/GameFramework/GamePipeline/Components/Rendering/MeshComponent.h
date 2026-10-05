#pragma once

#include <memory>
#include <vector>

#include "GamePipeline/Components/Base/Component.h"

class MeshAsset;
class MaterialAsset;
struct Skeleton;
class Mesh;

class MeshComponent : public Component
{
public:
	MeshComponent(Actor* aOwner);

	void Render() override;

	void SetMesh(const std::shared_ptr<MeshAsset>& aMeshAsset);

	const Mesh& GetMesh();
	Skeleton* GetSkeleton() const;

	void SetMaterial(unsigned int aMaterialIndex, const std::shared_ptr<MaterialAsset> aMaterialAsset);
	const std::vector<std::shared_ptr<MaterialAsset>>& GetMaterialAssetList() const;

private:
	std::shared_ptr<MeshAsset> myMeshAsset;
	std::vector<std::shared_ptr<MaterialAsset>> myMaterialAssets;
};


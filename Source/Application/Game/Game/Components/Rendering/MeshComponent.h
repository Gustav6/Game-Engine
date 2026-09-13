#pragma once

#include "Game/Components/Component.h"

#include "CommonUtilities/Collision/Sphere.hpp"

#include <AssetHandling/Asset/Types/MeshAsset.h>
#include <AssetHandling/Asset/Types/MaterialAsset.h>

#include <memory>
#include <vector>

struct Skeleton;
class Mesh;
class MaterialInterface;

class MeshComponent : public Component
{
public:
	MeshComponent(Actor* aOwner);

	void SetMesh(const std::shared_ptr<MeshAsset>& aMeshAsset);

	const Mesh& GetMesh();
	Skeleton* GetSkeleton() const;

	void SetMaterial(unsigned int aMaterialIndex, const std::shared_ptr<MaterialAsset> aMaterialAsset);
	const std::vector<std::shared_ptr<MaterialAsset>>& GetMaterialAssetList() const;

	CommonUtilities::Sphere<float> GetWorldSpaceBoundingSphere() const;
	CommonUtilities::Sphere<float> GetLocalSpaceBoundingSphere() const;

private:
	std::shared_ptr<MeshAsset> myMeshAsset;
	std::vector<std::shared_ptr<MaterialAsset>> myMaterialAssets;
};


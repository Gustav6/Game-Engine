#pragma once

#include "../Base/Component.h"

#include "CommonUtilities/Collision/Sphere.hpp"

#include <memory>
#include <vector>

class MeshAsset;
class MaterialAsset;
struct Skeleton;
class Mesh;

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


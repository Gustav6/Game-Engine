#pragma once

#include "Game/Component/Component.h"

#include "CommonUtilities/Sphere.hpp"

#include <AssetHandling/Asset/Types/MeshAsset.h>

#include <memory>
#include <vector>

struct Skeleton;
class Mesh;
class MaterialInterface;

class MeshComponent : public Component
{
public:
	MeshComponent(GameObject* aOwner);

	void SetMesh(const std::shared_ptr<MeshAsset>& aMeshAsset);

	const Mesh& GetMesh();
	Skeleton* GetSkeleton() const;

	void SetMaterial(unsigned int aMaterialIndex, const std::shared_ptr<MaterialInterface>& aMaterial);
	const std::vector<std::shared_ptr<MaterialInterface>>& GetMaterialList() const;

	CommonUtilities::Sphere<float> GetWorldSpaceBoundingSphere() const;
	CommonUtilities::Sphere<float> GetLocalSpaceBoundingSphere() const;

private:
	std::shared_ptr<MeshAsset> myMeshAsset;
	std::vector<std::shared_ptr<MaterialInterface>> myMaterials;
};


#pragma once

#include "Game/Component/Component.h"

#include "CommonUtilities/Sphere.hpp"

#include <memory>
#include <vector>

struct Skeleton;
class Mesh;
class MaterialInterface;

class MeshComponent : public Component
{
public:
	MeshComponent(GameObject* aOwner);

	std::shared_ptr<Mesh> GetMesh();
	void SetMesh(const std::shared_ptr<Mesh>& aMesh);

	Skeleton* GetSkeleton() const;

	void SetMaterial(unsigned int aMaterialIndex, const std::shared_ptr<MaterialInterface>& aMaterial);
	const std::vector<std::shared_ptr<MaterialInterface>>& GetMaterialList() const;

	CommonUtilities::Sphere<float> GetWorldSpaceBoundingSphere() const;
	CommonUtilities::Sphere<float> GetLocalSpaceBoundingSphere() const;

private:
	std::shared_ptr<Mesh> myMesh;
	std::vector<std::shared_ptr<MaterialInterface>> myMaterials;
};


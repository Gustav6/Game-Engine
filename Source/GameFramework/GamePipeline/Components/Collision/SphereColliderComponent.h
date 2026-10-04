#pragma once

#include <memory>

#include "GamePipeline/Components/Base/Component.h"

#include "CommonUtilities/Collision/Sphere.hpp"

class MeshAsset;

class SphereColliderComponent : public Component
{
public:
	SphereColliderComponent(Actor* aOwner);

	void Render() override;

	const CommonUtilities::Sphere<float>& GetCollider() const;

private:
	std::shared_ptr<MeshAsset> myDebugMesh;

	CommonUtilities::Sphere<float> myCollider;
};


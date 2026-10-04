#pragma once

#include <memory>

#include "GamePipeline/Components/Base/Component.h"

#include "CommonUtilities/Collision/AABB3D.hpp"

class MeshAsset;

class BoxColliderComponent : public Component
{
public:
	BoxColliderComponent(Actor* aOwner);

	void Render() override;

	const CommonUtilities::AABB3D<float>& GetCollider() const;

private:
	std::shared_ptr<MeshAsset> myDebugMesh;

	CommonUtilities::AABB3D<float> myCollider;
};


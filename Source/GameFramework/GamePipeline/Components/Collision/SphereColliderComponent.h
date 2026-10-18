#pragma once

#include <memory>

#include "GamePipeline/Components/Base/Component.h"

#include "CommonUtilities/Collision/Sphere.hpp"

class MeshAsset;

class SphereColliderComponent : public Component
{
public:
	SphereColliderComponent(Actor* aOwner);

#ifndef _RETAIL
	void Render() override;
#endif

	const CommonUtilities::Sphere<float>& GetSphere() const;
	CommonUtilities::Sphere<float> GetWorldSphere() const;
	
	void SetRadius(float aRadius);
	void SetOffset(const CommonUtilities::Vector3f& aOffset);
	void SetBounds(const CommonUtilities::Vector3f& aOffset, float aRadius);

private:
	CommonUtilities::Sphere<float> myCollider;
};


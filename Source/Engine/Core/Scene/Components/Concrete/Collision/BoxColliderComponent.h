#pragma once

#include "Core/Scene/Components/Interface/Component.h"

#include "CommonUtilities/Collision/AABB3D.hpp"

class BoxColliderComponent : public Component
{
public:
	BoxColliderComponent(Actor* aOwner);
	
#ifndef _RETAIL
	void Render() override;
#endif

	const CommonUtilities::AABB3D<float>& GetLocalAABB() const;
	CommonUtilities::AABB3D<float> GetWorldAABB() const;

	void SetSize(const CommonUtilities::Vector3f& aSize);
	void SetOffset(const CommonUtilities::Vector3f& aOffset);
	void SetBounds(const CommonUtilities::Vector3f& aCenter, const CommonUtilities::Vector3f& aSize);

private:
	CommonUtilities::AABB3D<float> myCollider;
};


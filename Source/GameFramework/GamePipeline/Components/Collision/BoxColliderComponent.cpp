#include "GameFramework.pch.h"
#include "BoxColliderComponent.h"

#include "GamePipeline/World/Actor.hpp"

#include "Managers/DebugSettingsManager.h"
#include "GraphicsEngine/Drawers/DebugDrawer.h"

BoxColliderComponent::BoxColliderComponent(Actor* aOwner)
	: Component(aOwner)
{ }

#ifndef _RETAIL
void BoxColliderComponent::Render()
{
	if (DebugSettingsManager::Get().GetSettings().ColliderDebugDrawEnabled)
	{
		const CommonUtilities::Vector3f worldScale = myOwner->GetTransform().GetScale();
		const CommonUtilities::Vector3f worldPosition = myOwner->GetTransform().GetPosition();

		const CommonUtilities::Vector3f minPoint = myCollider.GetMin() * worldScale + worldPosition;
		const CommonUtilities::Vector3f maxPoint = myCollider.GetMax() * worldScale + worldPosition;

		DebugDrawer::Get().DrawBox(minPoint, maxPoint, DebugSettingsManager::Get().GetSettings().ColliderColor);
	}
}
#endif

const CommonUtilities::AABB3D<float>& BoxColliderComponent::GetLocalAABB() const
{
	return myCollider;
}

CommonUtilities::AABB3D<float> BoxColliderComponent::GetWorldAABB() const
{
    // TODO: Translate to world space
	
	CommonUtilities::AABB3D<float> collider;
	
	return collider;
}

void BoxColliderComponent::SetSize(const CommonUtilities::Vector3f& aSize)
{
	const CommonUtilities::Vector3f halfExtents =
	{
		std::abs(aSize.x) * 0.5f,
		std::abs(aSize.y) * 0.5f,
		std::abs(aSize.z) * 0.5f
	};

	myCollider.InitWithMinAndMax(myCollider.GetCenter() - halfExtents, myCollider.GetCenter() + halfExtents);
}

void BoxColliderComponent::SetOffset(const CommonUtilities::Vector3f& aOffset)
{
    const CommonUtilities::Vector3f halfExtents = (myCollider.GetMax() - myCollider.GetMin()) * 0.5f;

    myCollider.InitWithMinAndMax(aOffset - halfExtents, aOffset + halfExtents);
}

void BoxColliderComponent::SetBounds(const CommonUtilities::Vector3f& aCenter, const CommonUtilities::Vector3f& aSize)
{
	const CommonUtilities::Vector3f halfExtents =
	{
		std::abs(aSize.x) * 0.5f,
		std::abs(aSize.y) * 0.5f,
		std::abs(aSize.z) * 0.5f
	};

	myCollider.InitWithMinAndMax(aCenter - halfExtents, aCenter + halfExtents);
}

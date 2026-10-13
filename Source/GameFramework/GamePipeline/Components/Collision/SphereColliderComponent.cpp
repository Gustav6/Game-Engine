#include "GameFramework.pch.h"
#include "SphereColliderComponent.h"

#include "GamePipeline/World/Actor.hpp"

#include "GraphicsEngine/Drawers/DebugDrawer.h"


SphereColliderComponent::SphereColliderComponent(Actor* aOwner)
	: Component(aOwner)
{ }

#ifdef _DEBUG
void SphereColliderComponent::Render()
{
	const CommonUtilities::Vector3f worldScale = myOwner->GetTransform().GetScale();
	const CommonUtilities::Vector3f worldPosition = myOwner->GetTransform().GetPosition();

	const float scale = std::max({ std::abs(worldScale.x), std::abs(worldScale.y), std::abs(worldScale.z) });

	const float radius = myCollider.GetRadius() * scale;

	const CommonUtilities::Vector3f position = myCollider.GetCenter() + worldPosition;

	DebugDrawer::Get().DrawSphere(radius, position, { 1.f, 1.f, 0.f, 1.f });
}
#endif

const CommonUtilities::Sphere<float>& SphereColliderComponent::GetSphere() const
{
	return myCollider;
}

CommonUtilities::Sphere<float> SphereColliderComponent::GetWorldSphere() const
{
    // TODO: Translate to world space
	CommonUtilities::Sphere<float> collider;
	
	return collider;
}

void SphereColliderComponent::SetRadius(float aRadius)
{
	myCollider.InitWithCenterAndRadius(myCollider.GetCenter(), aRadius);
}

void SphereColliderComponent::SetOffset(const CommonUtilities::Vector3f& aOffset)
{
	myCollider.InitWithCenterAndRadius(aOffset, myCollider.GetRadius());
}

void SphereColliderComponent::SetBounds(const CommonUtilities::Vector3f& aOffset, float aRadius)
{
	myCollider.InitWithCenterAndRadius(aOffset, aRadius);
}

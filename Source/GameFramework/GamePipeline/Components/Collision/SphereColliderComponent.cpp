#include "GameFramework.pch.h"
#include "SphereColliderComponent.h"

#include "GamePipeline/World/Actor.hpp"


SphereColliderComponent::SphereColliderComponent(Actor* aOwner)
	: Component(aOwner)
{ }

#ifdef _DEBUG
void SphereColliderComponent::Render()
{
	//const float scale = std::max({ std::abs(worldScale.x), std::abs(worldScale.y), std::abs(worldScale.z) });
	//const float radius = sphere.GetRadius() * scale;

	//CommonUtilities::Vector3f position = sphere.GetCenter() + worldPosition;

	//DebugDrawer::Get().DrawSphere(radius, position, { 1.f, 1.f, 0.f, 1.f });
}
#endif

const CommonUtilities::Sphere<float>& SphereColliderComponent::GetSphere() const
{
	return myCollider;
}

CommonUtilities::Sphere<float> SphereColliderComponent::GetWorldSphere() const
{
    // TODO: Translate to world space
	return myCollider;
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

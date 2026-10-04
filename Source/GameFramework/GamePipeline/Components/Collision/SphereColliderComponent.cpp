#include "GameFramework.pch.h"
#include "SphereColliderComponent.h"

#include "GamePipeline/World/Actor.hpp"


SphereColliderComponent::SphereColliderComponent(Actor* aOwner)
	: Component(aOwner)
{
	//myDebugMesh = ?
}

void SphereColliderComponent::Render()
{
	//DebugDrawCall drawCall =
	//{
	//	.Mesh = myDebugMesh,
	//	.Transform = myOwner->GetTransform().GetWorldMatrix()
	//};

	//DebugDrawer::Get().Submit(drawCall);
}

const CommonUtilities::Sphere<float>& SphereColliderComponent::GetCollider() const
{
	return myCollider;
}

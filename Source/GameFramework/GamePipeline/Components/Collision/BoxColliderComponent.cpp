#include "GameFramework.pch.h"
#include "BoxColliderComponent.h"

#include "GamePipeline/World/Actor.hpp"

BoxColliderComponent::BoxColliderComponent(Actor* aOwner)
	: Component(aOwner)
{
	//myDebugMesh = ?
}

void BoxColliderComponent::Render()
{
	//DebugDrawCall drawCall =
	//{
	//	.Mesh = myDebugMesh,
	//	.Transform = myOwner->GetTransform().GetWorldMatrix()
	//};

	//DebugDrawer::Get().Submit(drawCall);
}

const CommonUtilities::AABB3D<float>& BoxColliderComponent::GetCollider() const
{
	return myCollider;
}

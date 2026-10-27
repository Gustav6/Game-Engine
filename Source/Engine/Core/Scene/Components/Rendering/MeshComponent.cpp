#include "Core.pch.h"
#include "MeshComponent.h"

#include "AnimationComponent.h"

#include "Core/Assets/Registry/AssetRegistry.h"
#include "Core/Assets/Types/MeshAsset.h"
#include "Core/Assets/Types/MaterialAsset.h"

#include "Core/Scene/World/Actor.hpp"

#include "Graphics/Objects/Mesh/Mesh.h"

#include "Graphics/Drawers/MeshDrawer.h"
#include "Graphics/Drawers/DebugDrawer.h"

#include "Core/Managers/DebugSettingsManager.h"

#include "Ensure.h"

MeshComponent::MeshComponent(Actor* aOwner)
	: Component(aOwner)
{ }

void MeshComponent::Render()
{
	if (myMeshAsset == nullptr)
	{
		return;
	}

	MeshDrawCall drawCall =
	{
		.Mesh = &myMeshAsset->GetMesh(),
		.Transform = myOwner->GetTransform().GetWorldMatrix()
	};

	for (size_t i = 0; i < myMaterialAssets.size(); i++)
	{
		if (myMaterialAssets[i] != nullptr)
		{
			drawCall.Materials[i] = myMaterialAssets[i]->GetMaterial().get();
		}
	}

	AnimationComponent* animationComponent = myOwner->GetComponent<AnimationComponent>();

	if (animationComponent != nullptr)
	{
		drawCall.HasSkinning = true;
		drawCall.JointTransforms = animationComponent->GetJointTransforms();
	}

	MeshDrawer::Get().Submit(drawCall);

#ifndef _RETAIL
	if (DebugSettingsManager::Get().GetSettings().MeshDebugDrawBoxEnabled)
	{
		const CommonUtilities::Matrix4x4f& world = myOwner->GetTransform().GetWorldMatrix();
		const CommonUtilities::Vector3f worldScale = myOwner->GetTransform().GetScale();

		{
			const CommonUtilities::AABB3D<float>& box = myMeshAsset->GetMesh().GetBoundingBox();

			const CommonUtilities::Vector3f localMin = box.GetMin();
			const CommonUtilities::Vector3f localMax = box.GetMax();

			CommonUtilities::Vector3f minPoint = { FLT_MAX, FLT_MAX, FLT_MAX };
			CommonUtilities::Vector3f maxPoint = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

			for (int i = 0; i < 8; i++)
			{
				CommonUtilities::Vector4f local;

				if (i & 1)
				{
					local.x = localMax.x;
				}
				else
				{
					local.x = localMin.x;
				}

				if (i & 2)
				{
					local.y = localMax.y;
				}
				else
				{
					local.y = localMin.y;
				}

				if (i & 4)
				{
					local.z = localMax.z;
				}
				else
				{
					local.z = localMin.z;
				}

				local.w = 1.0f;

				const CommonUtilities::Vector4f transformedPoint = local * world;

				minPoint = { std::min(minPoint.x, transformedPoint.x), std::min(minPoint.y, transformedPoint.y), std::min(minPoint.z, transformedPoint.z) };
				maxPoint = { std::max(maxPoint.x, transformedPoint.x), std::max(maxPoint.y, transformedPoint.y), std::max(maxPoint.z, transformedPoint.z) };
			}

			DebugDrawer::Get().DrawBox(minPoint, maxPoint, DebugSettingsManager::Get().GetSettings().MeshBoundingBoxColor);
		}
	}

	if (DebugSettingsManager::Get().GetSettings().MeshDebugDrawSphereEnabled)
	{
		const CommonUtilities::Matrix4x4f& world = myOwner->GetTransform().GetWorldMatrix();
		const CommonUtilities::Vector3f worldScale = myOwner->GetTransform().GetScale();

		const CommonUtilities::Sphere<float>& sphere = myMeshAsset->GetMesh().GetBoundingSphere();

		const float scale = std::max({ std::abs(worldScale.x), std::abs(worldScale.y), std::abs(worldScale.z) });
		const float radius = sphere.GetRadius() * scale;

		const CommonUtilities::Vector4f local = sphere.GetCenter().ToXYZ1();

		const CommonUtilities::Vector4f transformedPoint = local * world;

		CommonUtilities::Vector3f position = transformedPoint.ToXYZ();

		DebugDrawer::Get().DrawSphere(radius, position, DebugSettingsManager::Get().GetSettings().MeshBoundingSphereColor);
	}
#endif
}

Skeleton* MeshComponent::GetSkeleton() const
{
	if (myMeshAsset == nullptr)
	{
		return nullptr;
	}

	return myMeshAsset->GetMesh().GetSkeleton();
}

void MeshComponent::SetMesh(const std::shared_ptr<MeshAsset>& aMeshAsset)
{
	myMeshAsset = aMeshAsset;
	myMaterialAssets.resize(myMeshAsset->GetMesh().GetNumMaterialSlots());
}

void MeshComponent::SetMaterial(unsigned int aMaterialIndex, const std::shared_ptr<MaterialAsset>& aMaterialAsset)
{
	ensure(aMaterialIndex < myMaterialAssets.size());

	myMaterialAssets[aMaterialIndex] = aMaterialAsset;
}
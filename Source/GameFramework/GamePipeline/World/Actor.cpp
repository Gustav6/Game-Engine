#include "GameFramework.pch.h"
#include "Actor.hpp"

#include <AssetPipeline/Assets/Types/MaterialAsset.h>
#include <AssetPipeline/Assets/Types/FontAsset.h>

#include <GraphicsEngine/Drawers/MeshDrawer.h>
#include <GraphicsEngine/Drawers/LightDrawer.h>
#include <GraphicsEngine/Drawers/TextDrawer.h>

#include <GamePipeline/Components/Rendering/TextComponent.h>
#include <GamePipeline/Components/Rendering/MeshComponent.h>
#include <GamePipeline/Components/Rendering/AnimationComponent.h>
#include <GamePipeline/Components/Light/DirectionlLightComponent.h>
#include <GamePipeline/Components/Light/PointLightComponent.h>
#include <GamePipeline/Components/Light/SpotLightComponent.h>

Actor::Actor()
{
	myIsEnabled = true;
}

Actor::~Actor() = default;

void Actor::Update(float aDeltaTime)
{ 
	for (auto& entry : myComponentLookup)
	{
		entry.second->Update(aDeltaTime);
	}
}

void Actor::Render()
{
	MeshComponent* meshComponent = GetComponent<MeshComponent>();

	if (meshComponent != nullptr)
	{
		MeshDrawCall drawCall =
		{
			.Mesh = &meshComponent->GetMesh(),
			.Transform = GetTransform().GetWorldMatrix()
		};

		const auto& materialAssets = meshComponent->GetMaterialAssetList();

		for (size_t i = 0; i < materialAssets.size(); i++)
		{
			drawCall.Materials[i] = materialAssets[i]->GetMaterial().get();
		}

		AnimationComponent* animationComponent = GetComponent<AnimationComponent>();

		if (animationComponent != nullptr)
		{
			drawCall.HasSkinning = true;
			drawCall.JointTransforms = animationComponent->GetJointTransforms();
		}

		MeshDrawer::Get().Submit(drawCall);
	}

	TextComponent* textComponent = GetComponent<TextComponent>();

	if (textComponent != nullptr)
	{
		TextDrawCall drawCall = 
		{
			.Text = &textComponent->GetText()
		};

		TextDrawer::Get().Submit(drawCall);
	}

	DirectionalLightComponent* directionalLight = GetComponent<DirectionalLightComponent>();

	if (directionalLight != nullptr)
	{
		directionalLight->UpdateLightViewProjection();

		SceneLight light =
		{
			.LightData = directionalLight->GetData(),
			.Transform = directionalLight->GetTransform(),
			.Projection = directionalLight->GetProjection()
		};

		LightDrawer::Get().Submit(light);
	}

	PointLightComponent* pointLight = GetComponent<PointLightComponent>();

	if (pointLight != nullptr)
	{
		SceneLight light =
		{
			.LightData = pointLight->GetData(),
			.ShadowMap = pointLight->GetShadowMap(),
			.Transform = pointLight->GetTransform(),
			.Projection = pointLight->GetProjection()
		};

		LightDrawer::Get().Submit(light);
	}

	SpotLightComponent* spotLight = GetComponent<SpotLightComponent>();

	if (spotLight != nullptr)
	{
		spotLight->UpdateLightViewProjection();

		SceneLight light =
		{
			.LightData = spotLight->GetData(),
			.ShadowMap = spotLight->GetShadowMap(),
			.Transform = spotLight->GetTransform(),
			.Projection = spotLight->GetProjection()
		};

		LightDrawer::Get().Submit(light);
	}
}

Transform& Actor::GetTransform()
{
	return myTransform;
}

void Actor::SetEnabled(bool aState)
{
	myIsEnabled = aState;
}

bool Actor::GetIsEnabled() const
{
	return myIsEnabled;
}

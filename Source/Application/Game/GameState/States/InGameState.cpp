#include "InGameState.h"

#include "Application.h"

#include "AssetPipeline/AssetRegistry/AssetRegistry.h"

#include "AssetPipeline/Assets/Types/MeshAsset.h"
#include "AssetPipeline/Assets/Types/AnimationAsset.h"
#include "AssetPipeline/Assets/Types/MaterialAsset.h"
#include "AssetPipeline/Assets/Types/TextureAsset.h"
#include "AssetPipeline/Assets/Types/FontAsset.h"
#include "AssetPipeline/Assets/Types/SpriteAsset.h"
#include "AssetPipeline/Assets/Types/UnrealSceneAsset.h"

#include "GamePipeline/World/Actor.hpp"
#include "GamePipeline/World/GameWorld.h"

#include "GamePipeline/Components/Rendering/MeshComponent.h"
#include "GamePipeline/Components/Rendering/AnimationComponent.h"
#include "GamePipeline/Components/Rendering/TextComponent.h"
#include "GamePipeline/Components/Rendering/SpriteComponent.h"
#include "GamePipeline/Components/Light/DirectionlLightComponent.h"
#include "GamePipeline/Components/Light/PointLightComponent.h"
#include "GamePipeline/Components/Light/SpotLightComponent.h"
#include "GamePipeline/Components/Collision/BoxColliderComponent.h"
#include "GamePipeline/Components/Collision/SphereColliderComponent.h"

#include "GraphicsEngine/GraphicsEngine.hpp"
#include "GraphicsEngine/Core/RenderEnums.h"

InGameState::InGameState()
{
	myAnimatedObject = nullptr;
	myDirectionLightObject = nullptr;
	myPointLightObject = nullptr;
	mySpotLightObject = nullptr;
	myEnableTestBinds = false;
}

void InGameState::Initialize()
{}

void InGameState::Enter()
{
	//LoadUnrealScene("LVL_DioramaBlockout_Level");
	LoadTestScene();
}

void InGameState::Update(float aDeltaTime)
{
	GameWorld::Get().Update(aDeltaTime);

	if (myEnableTestBinds)
	{
		UpdateTestBinds();
	}

	UpdateGraphicsDebugBinds();
}

void InGameState::Render()
{
	GameWorld::Get().Render();
}

void InGameState::Exit()
{
}

void InGameState::LoadUnrealScene(const char* aSceneName)
{
	std::shared_ptr<UnrealSceneAsset> scene = AssetRegistry::Get().GetAsset<UnrealSceneAsset>(aSceneName);

	if (scene != nullptr)
	{
		GameWorld::Get().AddActors(std::move(scene->GetSceneActors()));
	}
	else
	{
		SPDLOG_ERROR("Failed to load unreal scene [{}]!", aSceneName);
	}
}

void InGameState::LoadTestScene()
{
	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("SM_Color_Checker"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("SMColorChecker"));

		object->GetTransform().Translate({ -400, 40, 0 });

		GameWorld::Get().AddActor(std::move(object));
	}

	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("SM_Chest"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("ChestMaterial"));

		object->GetTransform().Rotate(CommonUtilities::Vector3f{ 0, -180, 0 });
		object->GetTransform().Translate({ 200, 0, 400 });

		GameWorld::Get().AddActor(std::move(object));
	}

	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Plane"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

		object->GetTransform().SetScale({ 2000, 2000, 2000 });

		GameWorld::Get().AddActor(std::move(object));
	}

	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Cube"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

		object->GetTransform().Translate({ 0, 50, 0 });
		object->GetTransform().SetScale({ 100, 100, 100 });

		GameWorld::Get().AddActor(std::move(object));
	}

	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Cube"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

		// TODO: Inspect rotation with multiple axis
		object->GetTransform().SetEulerRotation({ 45.0f, 0.0f, 0.0f });
		object->GetTransform().Translate({ -200, 100, 400 });
		object->GetTransform().SetScale({ 100, 100, 100 });

		GameWorld::Get().AddActor(std::move(object));
	}

	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Sphere"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

		object->GetTransform().Translate({ -200, 50, -200 });
		object->GetTransform().SetScale({ 100, 100, 100 });

		GameWorld::Get().AddActor(std::move(object));
	}

	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		object->GetTransform().Translate({ 200, 0, 0 });

		MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("SK_C_TGA_Bro"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("TGABroMaterial"));

		AnimationComponent* animationComponent = object->AddComponent<AnimationComponent>(meshComponent->GetSkeleton());

		AnimationEntry breathingAnimation =
		{
			.AnimationAsset = AssetRegistry::Get().GetAsset<AnimationAsset>("A_C_TGA_Bro_Idle_Breathing"),
			.ShouldLoop = true
		};
		AnimationEntry waveAnimation =
		{
			.AnimationAsset = AssetRegistry::Get().GetAsset<AnimationAsset>("A_C_TGA_Bro_Idle_Wave"),
			.ShouldLoop = false
		};
		AnimationEntry runAnimation =
		{
			.AnimationAsset = AssetRegistry::Get().GetAsset<AnimationAsset>("A_C_TGA_Bro_Run"),
			.ShouldLoop = true
		};
		AnimationEntry walkAnimation =
		{
			.AnimationAsset = AssetRegistry::Get().GetAsset<AnimationAsset>("A_C_TGA_Bro_Walk"),
			.ShouldLoop = true
		};

		animationComponent->AddAnimation("Breathing", breathingAnimation);
		animationComponent->AddAnimation("Wave", waveAnimation);
		animationComponent->AddAnimation("Run", runAnimation);
		animationComponent->AddAnimation("Walk", walkAnimation);

		BoxColliderComponent* boxColliderComponent = object->AddComponent<BoxColliderComponent>();

		boxColliderComponent->SetSize({ 64, 128, 64 });
		boxColliderComponent->SetOffset({ 0, 64, 0 });

		myAnimatedObject = GameWorld::Get().AddActor(std::move(object));
	}

	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		DirectionalLightComponent* directionalLight = object->AddComponent<DirectionalLightComponent>();

		directionalLight->SetColor({ 1.0f, 0.95f, 0.85f });
		directionalLight->SetIntensity(1.0f);
		directionalLight->SetDirection({ 0.0f, -0.75f, 1.0f });

		myDirectionLightObject = GameWorld::Get().AddActor(std::move(object));
	}

	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		object->Disable();

		PointLightComponent* pointLight = object->AddComponent<PointLightComponent>();

		pointLight->SetPosition({ -200.0f, 150.0f, 0.0f });
		pointLight->SetColor({ 1.0f, 0.95f, 0.85f });
		pointLight->SetIntensity(750000.0f);
		pointLight->SetRadius(500.0f);
		pointLight->SetShadowMapSize({ 512, 512 });

		myPointLightObject = GameWorld::Get().AddActor(std::move(object));
	}

	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		object->Disable();

		SpotLightComponent* spotLight = object->AddComponent<SpotLightComponent>();

		spotLight->SetPosition({ 0.0f, 350.0f, -200.0f });
		spotLight->SetColor({ 1.0f, 0.0f, 0.0f });
		spotLight->SetIntensity(10000000.0f);
		spotLight->SetRadius(400.0f);
		spotLight->SetDirection({ 0.0f, -1.0f, 0.0f });
		spotLight->SetCone(0.1f, 0.5f);
		spotLight->SetShadowMapSize({ 512, 512 });

		mySpotLightObject = GameWorld::Get().AddActor(std::move(object));
	}

	/*{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		TextComponent* text = object->AddComponent<TextComponent>();

		text->SetFont(AssetRegistry::Get().GetAsset<FontAsset>("CascadiaCode_M"));
		text->SetText("SHREK");
		text->SetSize(36);

		GameWorld::Get().AddActor(std::move(object));
	}*/

	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		SpriteComponent* sprite = object->AddComponent<SpriteComponent>();

		sprite->SetSprite(AssetRegistry::Get().GetAsset<SpriteAsset>("ShrekSprite"));
		sprite->SetSize({ 512, 512 });
		sprite->SetPosition({ 512, 512 });

		GameWorld::Get().AddActor(std::move(object));
	}

	myEnableTestBinds = true;
}

void InGameState::UpdateTestBinds()
{
	if (Application.InputManager.IsKeyPressed(KeyCode::D1))
	{
		myAnimatedObject->GetComponent<AnimationComponent>()->PlayAnimation("Walk", true);
		myAnimatedObject->GetComponent<AnimationComponent>()->PlayAnimation("Wave", false);
	}
	if (Application.InputManager.IsKeyPressed(KeyCode::D2))
	{
		myAnimatedObject->GetComponent<AnimationComponent>()->PlayAnimation("Run", true);
	}
	if (Application.InputManager.IsKeyPressed(KeyCode::D3))
	{
		myAnimatedObject->GetComponent<AnimationComponent>()->PlayAnimation("Breathing", true);
	}
	if (Application.InputManager.IsKeyPressed(KeyCode::D4))
	{
		if (myDirectionLightObject->GetIsEnabled())
		{
			myDirectionLightObject->Disable();
		}
		else
		{
			myDirectionLightObject->Enable();
		}
	}
	if (Application.InputManager.IsKeyPressed(KeyCode::D5))
	{
		if (myPointLightObject->GetIsEnabled())
		{
			myPointLightObject->Disable();
		}
		else
		{
			myPointLightObject->Enable();
		}
	}
	if (Application.InputManager.IsKeyPressed(KeyCode::D6))
	{
		if (mySpotLightObject->GetIsEnabled())
		{
			mySpotLightObject->Disable();
		}
		else
		{
			mySpotLightObject->Enable();
		}
	}
}

void InGameState::UpdateGraphicsDebugBinds()
{
	if (Application.InputManager.IsKeyPressed(KeyCode::F5))
	{
		int nextDebugRenderMode = static_cast<int>(GraphicsEngine::Get().GetSettings().DebugRenderMode) - 1;

		if (nextDebugRenderMode < 0)
		{
			nextDebugRenderMode = static_cast<int>(DebugRenderMode::MAX) - 1;
		}

		GraphicsEngine::Get().SetDebugRenderMode(static_cast<DebugRenderMode>(nextDebugRenderMode));
	}

	if (Application.InputManager.IsKeyPressed(KeyCode::F6))
	{
		int nextDebugRenderMode = static_cast<int>(GraphicsEngine::Get().GetSettings().DebugRenderMode) + 1;

		if (nextDebugRenderMode >= static_cast<int>(DebugRenderMode::MAX))
		{
			nextDebugRenderMode = 0;
		}

		GraphicsEngine::Get().SetDebugRenderMode(static_cast<DebugRenderMode>(nextDebugRenderMode));
	}

	if (Application.InputManager.IsKeyPressed(KeyCode::F7))
	{
		int nextToneMappingOperator = static_cast<int>(GraphicsEngine::Get().GetSettings().ToneMapper) + 1;

		if (nextToneMappingOperator >= static_cast<int>(ToneMapper::MAX))
		{
			nextToneMappingOperator = 0;
		}

		GraphicsEngine::Get().SetTonemap(static_cast<ToneMapper>(nextToneMappingOperator));
	}

	if (Application.InputManager.IsKeyPressed(KeyCode::F8))
	{
		GraphicsEngine::Get().SetBloomEnabled(!GraphicsEngine::Get().GetSettings().BloomEnabled);
	}
}

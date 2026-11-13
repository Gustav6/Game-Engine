#include "InGameState.h"

#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Math/MathUtils.hpp"

#include "Core/EngineContext.h"

#include "Core/Assets/Registry/AssetRegistry.h"

#include "Core/Assets/Concrete/MeshAsset.h"
#include "Core/Assets/Concrete/AnimationAsset.h"
#include "Core/Assets/Concrete/MaterialAsset.h"
#include "Core/Assets/Concrete/SpriteAsset.h"
#include "Core/Assets/Concrete/FontAsset.h"

#include "Core/Scene/World/Actor.hpp"
#include "Core/Scene/World/GameWorld.h"

#include "Core/Scene/Components/Concrete/Rendering/MeshComponent.h"
#include "Core/Scene/Components/Concrete/Rendering/AnimationComponent.h"
#include "Core/Scene/Components/Concrete/Rendering/TextComponent.h"
#include "Core/Scene/Components/Concrete/Rendering/SpriteComponent.h"
#include "Core/Scene/Components/Concrete/Rendering/InstancedMeshComponent.h"
#include "Core/Scene/Components/Concrete/Light/DirectionlLightComponent.h"
#include "Core/Scene/Components/Concrete/Light/PointLightComponent.h"
#include "Core/Scene/Components/Concrete/Light/SpotLightComponent.h"
#include "Core/Scene/Components/Concrete/Collision/BoxColliderComponent.h"
#include "Core/Scene/Components/Concrete/Collision/SphereColliderComponent.h"

#include "Graphics/GraphicsEngine.hpp"
#include "Graphics/Core/RenderEnums.h"

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

void InGameState::LoadTestScene()
{
	Actor* temp = nullptr;

	{
		Actor* actor = GameWorld::Get().CreateActor<Actor>("Instanced_Cubes");

		InstancedMeshComponent* meshComponent = actor->AddComponent<InstancedMeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Cube"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

		// Max mesh instances is set to 32 in the graphics engine.
		constexpr unsigned int instanceCount = 32;
		constexpr float radius = 2000.0f;

		for (unsigned int i = 0; i < instanceCount; i++)
		{
			const float angle = (2.0f * CommonUtilities::Math::PI<float> * i) / instanceCount;

			const float x = std::cos(angle) * radius;
			const float z = std::sin(angle) * radius;

			CommonUtilities::Matrix4x4f world;

			world.SetTranslation({ x, 100.0f, z });
			world.SetScaling({ 100, 100, 100 });

			InstanceData data =
			{
				.World = world,
				.WorldInvT = world.GetInverse().GetTranspose()
			};

			meshComponent->AddInstance(data);
		}
	}

	{
		Actor* actor = GameWorld::Get().CreateActor<Actor>("Color_Checker");

		MeshComponent* meshComponent = actor->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("SM_Color_Checker"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("SMColorChecker"));

		actor->GetTransform().Translate({ -400, 40, 0 });
	}

	{
		Actor* actor = GameWorld::Get().CreateActor<Actor>("Chest");

		MeshComponent* meshComponent = actor->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("SM_Chest"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("ChestMaterial"));

		actor->GetTransform().Rotate(CommonUtilities::Vector3f{ 0, -180, 0 });
		actor->GetTransform().Translate({ 200, 0, 400 });

		actor->SetParent(temp);
	}

	{
		Actor* actor = GameWorld::Get().CreateActor<Actor>("Unit_Plane");

		MeshComponent* meshComponent = actor->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Plane"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

		actor->GetTransform().SetScale({ 2000, 2000, 2000 });

		actor->SetParent(temp);
	}

	{
		Actor* actor = GameWorld::Get().CreateActor<Actor>("Unit_Cube");

		MeshComponent* meshComponent = actor->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Cube"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

		actor->GetTransform().Translate({ 0, 50, 0 });
		actor->GetTransform().SetScale({ 100, 100, 100 });
	}

	{
		Actor* actor = GameWorld::Get().CreateActor<Actor>("Unit_Cube");

		MeshComponent* meshComponent = actor->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Cube"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

		// TODO: Inspect rotation with multiple axis
		actor->GetTransform().SetEulerRotation({ 45.0f, 0.0f, 0.0f });
		actor->GetTransform().Translate({ -200, 100, 400 });
		actor->GetTransform().SetScale({ 100, 100, 100 });
	}

	{
		Actor* actor = GameWorld::Get().CreateActor<Actor>("Unit_Sphere");

		MeshComponent* meshComponent = actor->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Sphere"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

		actor->GetTransform().Translate({ -200, 50, -200 });
		actor->GetTransform().SetScale({ 100, 100, 100 });
	}

	{
		myAnimatedObject = GameWorld::Get().CreateActor<Actor>("TGA_Bro");

		myAnimatedObject->GetTransform().Translate({ 200, 0, 0 });

		MeshComponent* meshComponent = myAnimatedObject->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("SK_C_TGA_Bro"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("TGABroMaterial"));

		AnimationComponent* animationComponent = myAnimatedObject->AddComponent<AnimationComponent>(meshComponent->GetSkeleton());

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

		BoxColliderComponent* boxColliderComponent = myAnimatedObject->AddComponent<BoxColliderComponent>();

		boxColliderComponent->SetSize({ 64, 128, 64 });
		boxColliderComponent->SetOffset({ 0, 64, 0 });
	}

	{
		Actor* object = GameWorld::Get().CreateActor<Actor>("TGA_Bro_Temp");

		object->GetTransform().Translate({ 200, 0, -200 });

		MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

		meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("SK_C_TGA_Bro"));
		meshComponent->SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("TGABroMaterial"));
	}

	{
		myDirectionLightObject = GameWorld::Get().CreateActor<Actor>("Directional_Light");

		DirectionalLightComponent* directionalLight = myDirectionLightObject->AddComponent<DirectionalLightComponent>();

		directionalLight->SetColor({ 1.0f, 0.95f, 0.85f });
		directionalLight->SetIntensity(1.0f);
		directionalLight->SetDirection({ 0.0f, -0.75f, 1.0f });
	}

	{
		myPointLightObject = GameWorld::Get().CreateActor<Actor>("Point_Light");

		myPointLightObject->Disable();

		PointLightComponent* pointLight = myPointLightObject->AddComponent<PointLightComponent>();

		pointLight->SetPosition({ -200.0f, 150.0f, 0.0f });
		pointLight->SetColor({ 1.0f, 0.95f, 0.85f });
		pointLight->SetIntensity(750000.0f);
		pointLight->SetRadius(300.0f);
		pointLight->SetShadowMapSize({ 512, 512 });
	}

	// check out shadows from this
	{
		mySpotLightObject = GameWorld::Get().CreateActor<Actor>("Spot_Light");

		mySpotLightObject->Disable();

		SpotLightComponent* spotLight = mySpotLightObject->AddComponent<SpotLightComponent>();

		spotLight->SetPosition({ 0.0f, 350.0f, -200.0f });
		spotLight->SetColor({ 1.0f, 0.0f, 0.0f });
		spotLight->SetIntensity(10000000.0f);
		spotLight->SetRadius(400.0f);
		spotLight->SetDirection({ 0.0f, -1.0f, 0.0f });
		spotLight->SetCone(0.1f, 0.5f);
		spotLight->SetShadowMapSize({ 512, 512 });
	}

	/*{
		Actor* actor = GameWorld::Get().CreateActor<Actor>("Text");

		TextComponent* text = actor->AddComponent<TextComponent>();

		text->SetFont(AssetRegistry::Get().GetAsset<FontAsset>("CascadiaCode_M"));
		text->SetText("SHREK");
		text->SetSize(36);
	}

	{
		Actor* actor = GameWorld::Get().CreateActor<Actor>("Sprite");

		SpriteComponent* sprite = actor->AddComponent<SpriteComponent>();

		sprite->SetSprite(AssetRegistry::Get().GetAsset<SpriteAsset>("ShrekSprite"));
		sprite->SetSize({ 512, 512 });
		sprite->SetPosition({ 512, 512 });
	}*/

	myEnableTestBinds = true;
}

void InGameState::UpdateTestBinds()
{
	if (EngineContext.InputManager.IsKeyPressed(KeyCode::D1))
	{
		myAnimatedObject->GetComponent<AnimationComponent>()->PlayAnimation("Walk", true);
		myAnimatedObject->GetComponent<AnimationComponent>()->PlayAnimation("Wave", false);
	}
	if (EngineContext.InputManager.IsKeyPressed(KeyCode::D2))
	{
		myAnimatedObject->GetComponent<AnimationComponent>()->PlayAnimation("Run", true);
	}
	if (EngineContext.InputManager.IsKeyPressed(KeyCode::D3))
	{
		myAnimatedObject->GetComponent<AnimationComponent>()->PlayAnimation("Breathing", true);
	}
	if (EngineContext.InputManager.IsKeyPressed(KeyCode::D4))
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
	if (EngineContext.InputManager.IsKeyPressed(KeyCode::D5))
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
	if (EngineContext.InputManager.IsKeyPressed(KeyCode::D6))
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
	if (EngineContext.InputManager.IsKeyPressed(KeyCode::F5))
	{
		int nextDebugRenderMode = static_cast<int>(GraphicsEngine::Get().GetSettings().DebugRenderMode) - 1;

		if (nextDebugRenderMode < 0)
		{
			nextDebugRenderMode = static_cast<int>(DebugRenderMode::MAX) - 1;
		}

		GraphicsEngine::Get().SetDebugRenderMode(static_cast<DebugRenderMode>(nextDebugRenderMode));
	}

	if (EngineContext.InputManager.IsKeyPressed(KeyCode::F6))
	{
		int nextDebugRenderMode = static_cast<int>(GraphicsEngine::Get().GetSettings().DebugRenderMode) + 1;

		if (nextDebugRenderMode >= static_cast<int>(DebugRenderMode::MAX))
		{
			nextDebugRenderMode = 0;
		}

		GraphicsEngine::Get().SetDebugRenderMode(static_cast<DebugRenderMode>(nextDebugRenderMode));
	}

	if (EngineContext.InputManager.IsKeyPressed(KeyCode::F7))
	{
		int nextToneMappingOperator = static_cast<int>(GraphicsEngine::Get().GetSettings().ToneMapper) + 1;

		if (nextToneMappingOperator >= static_cast<int>(ToneMapper::MAX))
		{
			nextToneMappingOperator = 0;
		}

		GraphicsEngine::Get().SetTonemap(static_cast<ToneMapper>(nextToneMappingOperator));
	}

	if (EngineContext.InputManager.IsKeyPressed(KeyCode::F8))
	{
		GraphicsEngine::Get().SetBloomEnabled(!GraphicsEngine::Get().GetSettings().BloomEnabled);
	}
}

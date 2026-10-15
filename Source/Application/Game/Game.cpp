#include "Game.h"

#include <utility>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <memory>

#include "Application.h"

#include "AssetPipeline/AssetRegistry/AssetRegistry.h"

#include "AssetPipeline/Assets/Types/MeshAsset.h"
#include "AssetPipeline/Assets/Types/AnimationAsset.h"
#include "AssetPipeline/Assets/Types/MaterialAsset.h"
#include "AssetPipeline/Assets/Types/TextureAsset.h"
#include "AssetPipeline/Assets/Types/FontAsset.h"
#include "AssetPipeline/Assets/Types/SpriteAsset.h"

#include "GamePipeline/Components/Rendering/MeshComponent.h"
#include "GamePipeline/Components/Rendering/AnimationComponent.h"
#include "GamePipeline/Components/Rendering/TextComponent.h"
#include "GamePipeline/Components/Rendering/SpriteComponent.h"
#include "GamePipeline/Components/Light/DirectionlLightComponent.h"
#include "GamePipeline/Components/Light/PointLightComponent.h"
#include "GamePipeline/Components/Light/SpotLightComponent.h"
#include "GamePipeline/Components/Collision/BoxColliderComponent.h"
#include "GamePipeline/Components/Collision/SphereColliderComponent.h"

#include "GamePipeline/World/Actor.hpp"
#include "GamePipeline/World/GameWorld.h"

#include "GraphicsEngine/GraphicsEngine.hpp"
#include "GraphicsEngine/Core/RenderEnums.h"

#include "GraphicsEngine/Core/SceneCamera.h"
#include "GraphicsEngine/RHI/GraphicsCommandList.h"

#include "GamePipeline/Components/Registry/ComponentRegistryManager.h"

#include "Camera/GameCamera.h"
#include "GraphicsEngine/Core/RenderEnums.h"

#include "Logger/Logger.h"

#ifdef IMGUI_ENABLE
#include <ImGui/ImGuiLayer.h>
#include <ImGui/imgui.h>
#endif

Game::Game() = default;
Game::~Game() = default;

bool Game::Initialize(SIZE aWindowSize, WNDPROC aWindowProcess, LPCWSTR aWindowTitle)
{
	// TODO: Read or create a settings json file.
	// Should contain vsync flag and content directory.

	std::filesystem::path workingDirectory = GetExecutableDirectory().parent_path().parent_path() / "Content";;

	AssetRegistry::Get().RegisterAssetType<MeshAsset>({ ".fbx" });
	AssetRegistry::Get().RegisterAssetType<AnimationAsset>({ ".fbx" });
	AssetRegistry::Get().RegisterAssetType<MaterialAsset>({ ".json" });
	AssetRegistry::Get().RegisterAssetType<TextureAsset>({ ".dds" });
	AssetRegistry::Get().RegisterAssetType<FontAsset>({ ".json" });
	AssetRegistry::Get().RegisterAssetType<SpriteAsset>({ ".dds" });

	if (!AssetRegistry::Get().Initialize(workingDirectory))
	{
		throw std::runtime_error("AssetRegistry failed to initialize!");
	}

	Application.InputManager.SetWindowHandle(myMainWindowHandle);

	constexpr LPCWSTR windowClassName = L"ModelViewerMainWindow";

	// First we create our Window Class
	WNDCLASS windowClass = {};
	windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = aWindowProcess;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.lpszClassName = windowClassName;
	RegisterClass(&windowClass);

	// Put the window in the middle of the screen regardless of resolution.
	LONG posX = (GetSystemMetrics(SM_CXSCREEN) - aWindowSize.cx) / 2;
	posX = std::max<LONG>(posX, 0);

	LONG posY = (GetSystemMetrics(SM_CYSCREEN) - aWindowSize.cy) / 2;
	posY = std::max<LONG>(posY, 0);

	// Then we use the class to create our window
	myMainWindowHandle = CreateWindow(
		windowClassName,                                // Classname
		aWindowTitle,                                   // Window Title
		WS_OVERLAPPEDWINDOW | WS_POPUP,                 // Flags
		posX,
		posY,
		aWindowSize.cx,
		aWindowSize.cy,
		nullptr, nullptr, nullptr,
		nullptr
	);

	spdlog::info("Ready!");

	// Graphics Init
	{
		spdlog::info("Initializing Graphics Engine...");

		GraphicsEngine& GE = GraphicsEngine::Get();

		if (!GE.Initialize(myMainWindowHandle, workingDirectory))
		{
			throw std::runtime_error("Graphics Engine failed to initialize!");
		}

#ifdef IMGUI_ENABLE
		myImGuiLayer = std::make_unique<ImGuiLayer>(myMainWindowHandle);
		GE.SetRenderOverlay(myImGuiLayer.get());
#endif

		std::shared_ptr<TextureAsset> environmentTexture = AssetRegistry::Get().GetAsset<TextureAsset>("T_Shipyard_cm");

		if (environmentTexture != nullptr)
		{
			GE.SetEnvironmentTexture(environmentTexture->GetTexture());
		}

		myGraphicsCommandList = std::make_unique<GraphicsCommandList>();

		GE.CreateCommandList("ModelViewer", *myGraphicsCommandList);
	}

	LoadScene();

	// Show our program window and give it focus.
	ShowWindow(myMainWindowHandle, SW_SHOW);
	SetForegroundWindow(myMainWindowHandle);

	return true;
}

int Game::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	myIsRunning = true;

	while (myIsRunning)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			Application.InputManager.UpdateEvents(msg.message, msg.wParam, msg.lParam);

			switch (msg.message)
			{
				case WM_QUIT:
				{
					myIsRunning = false;
					break;
				}
				default:
					break;
			}
		}

		Application.TimeManager.Update();
		Application.InputManager.UpdateInput();

		float deltaTime = Application.TimeManager.GetDeltaTime();

		UpdateScene(deltaTime);

		// Render to graphics engine and present to screen
		{
			GraphicsEngine& GE = GraphicsEngine::Get();

			GE.BeginFrame();

			myGraphicsCommandList->ResetCommandList();

			RenderScene();

			SceneCamera camera =
			{
				.Transform = GameCamera::GetInstance().GetTransform().GetWorldMatrix(),
				.RenderCamera = GameCamera::GetInstance().GetRenderCamera()
			};

			FrameStats frameStats = GE.Render(*myGraphicsCommandList, camera);
			
#ifdef IMGUI_ENABLE
			ImGui::PushFont(nullptr, 28.0f);
			ImGui::Begin("Debug");

			if (ImGui::CollapsingHeader("Frame Info", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
				ImGui::Text("DrawCallCount: %u", frameStats.DrawCallCount);
				ImGui::Text("PipelineSwitches: %u", frameStats.PipelineSwitches);
				ImGui::Text("VisibleObjects: %u", frameStats.VisibleObjects);
				ImGui::Text("CulledObjects: %u", frameStats.CulledObjects);
			}

			if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen))
			{
				RenderSettings settings = GraphicsEngine::Get().GetSettings();

				switch (settings.DebugRenderMode)
				{
					case DebugRenderMode::None:
					{
						ImGui::Text("DebugRenderMode: None");
						break;
					}
					case DebugRenderMode::sRGB:
					{
						ImGui::Text("DebugRenderMode: SRGB");
						break;
					}
					case DebugRenderMode::Roughness:
					{
						ImGui::Text("DebugRenderMode: Roughness");
						break;
					}
					case DebugRenderMode::Metalness:
					{
						ImGui::Text("DebugRenderMode: Metalness");
						break;
					}
					case DebugRenderMode::AmbientOcclusionTexture:
					{
						ImGui::Text("DebugRenderMode: AO (Texture)");
						break;
					}
					case DebugRenderMode::AmbientOcclusionScreenSpace:
					{
						ImGui::Text("DebugRenderMode: AO (Screen Space)");
						break;
					}
					case DebugRenderMode::NormalsTangentSpace:
					{
						ImGui::Text("DebugRenderMode: Normals (Tangent Space)");
						break;
					}
					case DebugRenderMode::NormalsWorldSpace:
					{
						ImGui::Text("DebugRenderMode: Normals (World Space)");
						break;
					}
					case DebugRenderMode::Shadows:
					{
						ImGui::Text("DebugRenderMode: Shadows");
						break;
					}
					default:
					{
						break;
					}
				}

				switch (settings.ToneMapper)
				{
					case ToneMapper::None:
					{
						ImGui::Text("ToneMapper: None");
						break;
					}
					case ToneMapper::Aces:
					{
						ImGui::Text("ToneMapper: Aces");
						break;
					}
					case ToneMapper::Lottes:
					{
						ImGui::Text("ToneMapper: Lottes");
						break;
					}
					case ToneMapper::Unreal:
					{
						ImGui::Text("ToneMapper: Unreal");
						break;
					}
					default:
					{
						break;
					}
				}

				if (settings.BloomEnabled)
				{
					ImGui::Text("Bloom: Enabled");
				}
				else
				{
					ImGui::Text("Bloom: Disabled");
				}

				if (settings.VSyncEnabled)
				{
					ImGui::Text("VSync: Enabled");
				}
				else
				{
					ImGui::Text("VSync: Disabled");
				}
			}

			ImGui::End();
			ImGui::PopFont();
#endif
			myGraphicsCommandList->FinishCommandList();

			GE.ExecuteCommandList(*myGraphicsCommandList);
			GE.Present();
		}
	}

	// Cleanup
	AssetRegistry::Get().UnInitialize();
	ComponentRegistryManager::Get().Shutdown();

	return 0;
}

void Game::LoadScene()
{
	{
		GameCamera& gameCamera = GameCamera::GetInstance();
		gameCamera.Init(90.0f, 1.0f, 10000.0f, GraphicsEngine::Get().GetClientSize());
	}

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

		object->GetTransform().Translate({ 0, 50, -200 });
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
	    pointLight->SetRadius(900.0f);
	    pointLight->SetShadowMapSize({ 512, 512 });

	    myPointLightObject = GameWorld::Get().AddActor(std::move(object));
	}

	{
	    std::unique_ptr<Actor> object = std::make_unique<Actor>();

	    object->Disable();

	    SpotLightComponent* spotLight = object->AddComponent<SpotLightComponent>();

	    spotLight->SetPosition({ 0.0f, 50.0f, -200.0f });
		spotLight->SetColor({ 1.0f, 0.95f, 0.85f });
	    spotLight->SetIntensity(1000000.0f);
	    spotLight->SetRadius(1000.0f);
	    spotLight->SetDirection({ 0.0f, 0.0f, 1.0f });
	    spotLight->SetCone(0.0f, 0.7f);
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

	/*{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		SpriteComponent* sprite = object->AddComponent<SpriteComponent>();

		sprite->SetSprite(AssetRegistry::Get().GetAsset<SpriteAsset>("ShrekSprite"));
		sprite->SetSize({ 512, 512 });
		sprite->SetPosition({ 512, 512 });

		GameWorld::Get().AddActor(std::move(object));
	}*/
}

void Game::UpdateScene(float aDeltaTime)
{
	GameWorld::Get().Update(aDeltaTime);
	GameCamera::GetInstance().Update(aDeltaTime);

	// TODO: TEMP
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

	if (Application.InputManager.IsKeyPressed(KeyCode::F5))
	{
		GraphicsEngine::Get().CycleDebugRenderMode(-1);
	}
	if (Application.InputManager.IsKeyPressed(KeyCode::F6))
	{
		GraphicsEngine::Get().CycleDebugRenderMode(1);
	}
	if (Application.InputManager.IsKeyPressed(KeyCode::F7))
	{
		GraphicsEngine::Get().CycleToNextTonemap(1);
	}
	if (Application.InputManager.IsKeyPressed(KeyCode::F8))
	{
		GraphicsEngine::Get().ToggleBloom();
	}

	if (Application.InputManager.IsKeyReleased(KeyCode::Escape))
	{
		PostQuitMessage(0);
	}
}

void Game::RenderScene()
{
	GameWorld::Get().Render();
}

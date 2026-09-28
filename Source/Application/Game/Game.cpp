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
#include "GamePipeline/World/Actor.hpp"
#include "GamePipeline/World/GameWorld.h"

#include "GraphicsEngine/GraphicsEngine.hpp"

#include "GraphicsEngine/Core/SceneCamera.h"
#include "GraphicsEngine/Objects/GPU/Texture.h"
#include "GraphicsEngine/RHI/GraphicsCommandList.h"

#include "GamePipeline/Components/Registry/ComponentRegistryManager.h"

#include "Camera/GameCamera.h"

#include <tracy/core/Tracy.hpp>

Game::Game() = default;
Game::~Game() = default;

bool Game::Initialize(SIZE aWindowSize, WNDPROC aWindowProcess, LPCWSTR aWindowTitle)
{
	std::filesystem::path workingDirectory = GetExecutableDirectory().parent_path().parent_path() / "Content";;

	AssetRegistry::Get().RegisterAssetType<MeshAsset>({ ".fbx" });
	AssetRegistry::Get().RegisterAssetType<AnimationAsset>({ ".fbx" });
	AssetRegistry::Get().RegisterAssetType<MaterialAsset>({ ".json" });
	AssetRegistry::Get().RegisterAssetType<TextureAsset>({ ".dds" });
	AssetRegistry::Get().RegisterAssetType<FontAsset>({ ".json" });
	AssetRegistry::Get().RegisterAssetType<SpriteAsset>({ ".dds" });

	if (!AssetRegistry::Get().Initialize(workingDirectory))
	{
		return false;
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

#if _DEBUG
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
#else

	aWindowSize;

	MONITORINFO monitorInfo = {};
	monitorInfo.cbSize = sizeof(MONITORINFO);

	HMONITOR monitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
	GetMonitorInfo(monitor, &monitorInfo);

	const RECT& monitorRect = monitorInfo.rcMonitor;

	const LONG posX = monitorRect.left;
	const LONG posY = monitorRect.top;
	const LONG width = monitorRect.right - monitorRect.left;
	const LONG height = monitorRect.bottom - monitorRect.top;

	// Then we use the class to create our window
	myMainWindowHandle = CreateWindow(
		windowClassName,                                // Classname
		aWindowTitle,                                   // Window Title
		/*WS_THICKFRAME |*/ WS_POPUP,                 // Flags
		posX,
		posY,
		width,
		height,
		nullptr, nullptr, nullptr,
		nullptr
	);
#endif

	MVLOG(Log, "Ready!");

	// Graphics Init
	{
		MVLOG(Log, "Initializing Graphics Engine...");

		GraphicsEngine& GE = GraphicsEngine::Get();

		if (!GE.Initialize(myMainWindowHandle, workingDirectory))
		{
			return false;
		}

		std::unique_ptr<Texture> environmentTexture = std::make_unique<Texture>();

		GE.CreateTexture(workingDirectory / "Textures" / "Environment Map" / "T_Shipyard.dds", *environmentTexture);
		GE.SetEnvironmentTexture(std::move(environmentTexture));

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
				{
					break;
				}
			}
		}

		Application.TimeManager.Update();
		Application.InputManager.UpdateInput();

		float deltaTime = Application.TimeManager.GetDeltaTime();

		UpdateScene(deltaTime);

		myFpsCounter->GetComponent<TextComponent>()->SetText(std::format("FPS : {}", static_cast<int>(1.0f / deltaTime)));

		// Render to graphics engine and present to screen
		{
			GraphicsEngine& GE = GraphicsEngine::Get();

			GE.BeginFrame();

			RenderScene();

			SceneCamera camera =
			{
				.Transform = GameCamera::GetInstance().GetTransform().GetWorldMatrix(),
				.RenderCamera = &GameCamera::GetInstance().GetRenderCamera()
			};

			myGraphicsCommandList->ResetCommandList();

			GE.Render(*myGraphicsCommandList, camera);

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

	    MeshComponent& meshComponent = object->AddComponent<MeshComponent>();

	    meshComponent.SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("SM_Color_Checker"));
	    meshComponent.SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("SMColorChecker"));

	    object->GetTransform().Translate({ -400, 40, 0 });
	    object->GetTransform().Rotate(CommonUtilities::Vector3f{ -90, 0, 0 });

		GameWorld::Get().AddActor(std::move(object));
	}

	{
	    std::unique_ptr<Actor> object = std::make_unique<Actor>();

		MeshComponent& meshComponent = object->AddComponent<MeshComponent>();

	    meshComponent.SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("SM_Chest"));
	    meshComponent.SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("ChestMaterial"));

	    object->GetTransform().Rotate(CommonUtilities::Vector3f{ 0, -180, 0 });
	    object->GetTransform().Translate({ 200, 0, 400 });

		GameWorld::Get().AddActor(std::move(object));
	}

	{
	    std::unique_ptr<Actor> object = std::make_unique<Actor>();

		MeshComponent& meshComponent = object->AddComponent<MeshComponent>();

	    meshComponent.SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Plane"));
	    meshComponent.SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

	    object->GetTransform().SetScale({ 2000, 2000, 2000 });

		GameWorld::Get().AddActor(std::move(object));
	}

	{
	    std::unique_ptr<Actor> object = std::make_unique<Actor>();

		MeshComponent& meshComponent = object->AddComponent<MeshComponent>();

	    meshComponent.SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Cube"));
	    meshComponent.SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

	    object->GetTransform().Translate({ 0, 50, 0 });
	    object->GetTransform().SetScale({ 100, 100, 100 });

		GameWorld::Get().AddActor(std::move(object));
	}

	{
	    std::unique_ptr<Actor> object = std::make_unique<Actor>();

		MeshComponent& meshComponent = object->AddComponent<MeshComponent>();

	    meshComponent.SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("Unit_Cube"));
	    meshComponent.SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("BaseMaterial"));

	    object->GetTransform().SetEulerRotation({ 45.0f, 180.0f, 45.0f });
	    object->GetTransform().Translate({ -200, 100, 400 });
	    object->GetTransform().SetScale({ 100, 100, 100 });

		GameWorld::Get().AddActor(std::move(object));
	}

	{
	    std::unique_ptr<Actor> object = std::make_unique<Actor>();

	    object->GetTransform().Translate({ 200, 0, 0 });

		MeshComponent& meshComponent = object->AddComponent<MeshComponent>();

	    meshComponent.SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>("SK_C_TGA_Bro"));
	    meshComponent.SetMaterial(0, AssetRegistry::Get().GetAsset<MaterialAsset>("TGABroMaterial"));

	    AnimationComponent& animationComponent = object->AddComponent<AnimationComponent>(meshComponent.GetSkeleton());

	    AnimationEntry breathingAnimation =
	    {
	        .AnimationAsset = AssetRegistry::Get().GetAsset<AnimationAsset>("A_C_TGA_Bro_Idle_Breathing"),
	        .ShouldLoop = true
	    };
	    AnimationEntry waveAnimation =
	    {
	        .AnimationAsset = AssetRegistry::Get().GetAsset<AnimationAsset>("A_C_TGA_Bro_Idle_Wave"),
	        .ShouldLoop = true
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

	    animationComponent.AddAnimation("Breathing", breathingAnimation);
	    animationComponent.AddAnimation("Wave", waveAnimation);
	    animationComponent.AddAnimation("Run", runAnimation);
	    animationComponent.AddAnimation("Walk", walkAnimation);
	    
	    myAnimatedObject = GameWorld::Get().AddActor(std::move(object));
	}

	{
	    std::unique_ptr<Actor> object = std::make_unique<Actor>();

	    DirectionalLightComponent& directionalLight = object->AddComponent<DirectionalLightComponent>();

	    directionalLight.SetColor({ 1.0f, 0.95f, 0.85f });
	    directionalLight.SetIntensity(1.0f);
	    directionalLight.SetDirection({ 0.0f, -0.75f, 1.0f });

	    myDirectionLightObject = GameWorld::Get().AddActor(std::move(object));
	}

	{
	    std::unique_ptr<Actor> object = std::make_unique<Actor>();

	    object->SetEnabled(false);

	    PointLightComponent& pointLight = object->AddComponent<PointLightComponent>();

	    pointLight.SetPosition({ -200.0f, 150.0f, 0.0f });
		pointLight.SetColor({ 1.0f, 0.95f, 0.85f });
	    pointLight.SetIntensity(750000.0f);
	    pointLight.SetRadius(900.0f);
	    pointLight.SetShadowMapSize({ 512, 512 });

	    myPointLightObject = GameWorld::Get().AddActor(std::move(object));
	}

	{
	    std::unique_ptr<Actor> object = std::make_unique<Actor>();

	    object->SetEnabled(false);

	    SpotLightComponent& spotLight = object->AddComponent<SpotLightComponent>();

	    spotLight.SetPosition({ 0.0f, 50.0f, -200.0f });
		spotLight.SetColor({ 1.0f, 0.95f, 0.85f });
	    spotLight.SetIntensity(1000000.0f);
	    spotLight.SetRadius(1000.0f);
	    spotLight.SetDirection({ 0.0f, 0.0f, 1.0f });
	    spotLight.SetCone(0.0f, 0.7f);
	    spotLight.SetShadowMapSize({ 512, 512 });

	    mySpotLightObject = GameWorld::Get().AddActor(std::move(object));
	}

	{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		TextComponent& text = object->AddComponent<TextComponent>();

		text.SetFont(AssetRegistry::Get().GetAsset<FontAsset>("CascadiaCode_M"));
		text.SetText("SHREK");
		text.SetSize(36);

		myFpsCounter = GameWorld::Get().AddActor(std::move(object));
	}

	/*{
		std::unique_ptr<Actor> object = std::make_unique<Actor>();

		SpriteComponent& sprite = object->AddComponent<SpriteComponent>();

		sprite.SetSprite(AssetRegistry::Get().GetAsset<SpriteAsset>("ShrekSprite"));
		sprite.SetSize({ 512, 512 });
		sprite.SetPosition({ 512, 512 });

		GameWorld::Get().AddActor(std::move(object));
	}*/
}

void Game::UpdateScene(float aDeltaTime)
{
	ZoneScopedN("Update Game");

	GameWorld::Get().Update(aDeltaTime);
	GameCamera::GetInstance().Update(aDeltaTime);

	// TODO: TEMP
	if (Application.InputManager.IsKeyPressed(KeyCode::D1))
	{
		myAnimatedObject->GetComponent<AnimationComponent>()->PlayAnimation("Wave", true, JointGroup::RightArm);
		myAnimatedObject->GetComponent<AnimationComponent>()->PlayAnimation("Walk", false);
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
		myDirectionLightObject->SetEnabled(!myDirectionLightObject->GetIsEnabled());
	}
	if (Application.InputManager.IsKeyPressed(KeyCode::D5))
	{
		myPointLightObject->SetEnabled(!myPointLightObject->GetIsEnabled());
	}
	if (Application.InputManager.IsKeyPressed(KeyCode::D6))
	{
		mySpotLightObject->SetEnabled(!mySpotLightObject->GetIsEnabled());
	}

	if (Application.InputManager.IsKeyPressed(KeyCode::F5))
	{
		GraphicsEngine::Get().CycleToNextRenderPass(-1);
	}
	if (Application.InputManager.IsKeyPressed(KeyCode::F6))
	{
		GraphicsEngine::Get().CycleToNextRenderPass(1);
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

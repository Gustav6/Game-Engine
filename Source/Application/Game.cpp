#include "Game.h"

#include <utility>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <memory>

#include "Core/EngineContext.h"

#include "Core/Assets/Registry/AssetRegistry.h"

#include "Core/Assets/Concrete/MeshAsset.h"
#include "Core/Assets/Concrete/AnimationAsset.h"
#include "Core/Assets/Concrete/MaterialAsset.h"
#include "Core/Assets/Concrete/TextureAsset.h"
#include "Core/Assets/Concrete/FontAsset.h"
#include "Core/Assets/Concrete/SpriteAsset.h"

#include "GameState/GameStateMachine.h"

#include "Core/Scene/World/Actor.hpp"
#include "Core/Scene/World/GameWorld.h"
#include "Core/Scene/Components/Registry/ComponentRegistryManager.h"

#include "Graphics/GraphicsEngine.hpp"
#include "Graphics/Core/RenderEnums.h"

#include "Graphics/Core/SceneCamera.h"
#include "Graphics/RHI/GraphicsCommandList.h"

#include "Core/Managers/DebugSettingsManager.h"
#include "Core/Managers/WindowManager.h"

#include "Camera/GameCamera.h"

#include "Wrappers/FileWatcher/FileWatcherWrapper.h"
#include "Wrappers/Json/JsonWrapper.h"
#include "Wrappers/Logger/LoggerWrapper.h"

#ifndef _RETAIL
#include <Tools/Core/ToolRegistry.h>
#include <Tools/Core/ToolManager.h>

#include <Tools/Custom/Debug/DebugTool.h>
#include <Tools/Editor/Hierarchy/SceneHierarchy.h>

#include <Tools/ImGui/ImGuiLayer.h>
#include <ImGui/imgui.h>
#endif

Game::Game() = default;
Game::~Game() = default;

bool Game::Initialize(SIZE aWindowSize, WNDPROC aWindowProcess, LPCWSTR aWindowTitle)
{
	const std::filesystem::path settingsPath = GetExecutableDirectory() / "Settings.json";

	if (!std::filesystem::exists(settingsPath))
	{
		Json::Writer writer = Json::Write(settingsPath, Json::WriteFlag_Pretty);

		if (!writer.SetString("ContentRoot", "../../Content"))
		{
			throw std::runtime_error("Failed to create settings file!");
		}
		if (!writer.SetString("ShaderRoot", "../../Shaders"))
		{
			throw std::runtime_error("Failed to create settings file!");
		}

		writer.Save();
	}

	Json::Data settings = Json::Read(settingsPath);

	if (settings.HasKey("ContentRoot"))
	{
		EngineContext.ContentRoot = GetExecutableDirectory()/ settings.GetString("ContentRoot");
	}
	else
	{
		throw std::runtime_error("EngineContext settings has no content root set!");
	}

	if (settings.HasKey("ShaderRoot"))
	{
		EngineContext.ShaderRoot = GetExecutableDirectory() / settings.GetString("ShaderRoot");
	}
	else
	{
		throw std::runtime_error("EngineContext settings has no shader root set!");
	}

	AssetRegistry::Get().RegisterAssetType<MeshAsset>({ ".fbx" });
	AssetRegistry::Get().RegisterAssetType<AnimationAsset>({ ".fbx" });
	AssetRegistry::Get().RegisterAssetType<MaterialAsset>({ ".json" });
	AssetRegistry::Get().RegisterAssetType<TextureAsset>({ ".dds" });
	AssetRegistry::Get().RegisterAssetType<FontAsset>({ ".dds",".json" });
	AssetRegistry::Get().RegisterAssetType<SpriteAsset>({ ".dds" });

	if (!AssetRegistry::Get().Initialize())
	{
		throw std::runtime_error("AssetRegistry failed to initialize!");
	}

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

	EngineContext.InputManager.SetWindowHandle(myMainWindowHandle);
	WindowManager::Get().SetWindowHandle(myMainWindowHandle);

	SPDLOG_INFO("Ready!");

	// Graphics Initialize
	{
		SPDLOG_INFO("Initializing Graphics Engine...");

		GraphicsEngine& GE = GraphicsEngine::Get();

		if (!GE.Initialize(myMainWindowHandle, EngineContext.ShaderRoot))
		{
			throw std::runtime_error("Graphics Engine failed to initialize!");
		}

		myGraphicsCommandList = std::make_unique<GraphicsCommandList>();

		GE.CreateCommandList("Application", *myGraphicsCommandList);

		std::shared_ptr<TextureAsset> environmentTexture = AssetRegistry::Get().GetAsset<TextureAsset>("T_Shipyard_cm");

		if (environmentTexture != nullptr)
		{
			GE.SetEnvironmentTexture(environmentTexture->GetTexture());
		}

#ifndef _RETAIL
		myImGuiLayer = std::make_unique<ImGuiLayer>(myMainWindowHandle);
		GE.SetRenderOverlay(myImGuiLayer.get());

		ToolRegistry::Get().Initialize();

		ToolManager::Get().AddTool<DebugTool>();
		//ToolManager::Get().AddTool<SceneHierarchy>();
#endif
	}

	{
		myGameCamera = GameWorld::Get().CreateActor<GameCamera>("Game_Camera");
		myGameCamera->Init(90.0f, 1.0f, 10000.0f, GraphicsEngine::Get().GetClientSize());
	}

	{
		myStateMachine = std::make_unique<GameStateMachine>();
		myStateMachine->Initialize();
		myStateMachine->ChangeState(GameStateType::InGame);
	}

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

			EngineContext.InputManager.UpdateEvents(msg.message, msg.wParam, msg.lParam);

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

		EngineContext.TimeManager.Update();
		EngineContext.InputManager.UpdateInput();

		const float deltaTime = EngineContext.TimeManager.GetDeltaTime();

		UpdateScene(deltaTime);

		// Render to graphics engine and present to screen
		{
			GraphicsEngine& GE = GraphicsEngine::Get();

			GE.BeginFrame();

			myGraphicsCommandList->ResetCommandList();

			RenderScene();

			SceneCamera camera =
			{
				.Transform = myGameCamera->GetTransform().GetWorldMatrix(),
				.RenderCamera = myGameCamera->GetRenderCamera()
			};

			FrameStats frameStats = GE.Render(*myGraphicsCommandList, camera);

#ifndef _RETAIL
			const DebugInfo info = 
			{
				.FPS = 1.0f / deltaTime,
				.FrameTime = deltaTime,
				.DrawCallCount = frameStats.DrawCallCount,
				.PipelineSwitches = frameStats.PipelineSwitches,
				.VisibleObjects = frameStats.VisibleObjects,
				.CulledObjects = frameStats.CulledObjects
			};

			ToolManager::Get().GetTool<DebugTool>()->SetInfo(info);

			ToolManager::Get().Render();
#endif
	
			myGraphicsCommandList->FinishCommandList();

			GE.ExecuteCommandList(*myGraphicsCommandList);
			GE.Present();
		}

		FileWatcher::DispatchEvents();
	}

	// Cleanup
	AssetRegistry::Get().Shutdown();
	ComponentRegistryManager::Get().Shutdown();
	GraphicsEngine::Get().Shutdown();

	return 0;
}

void Game::UpdateScene(float aDeltaTime) const
{
	myStateMachine->Update(aDeltaTime);
}

void Game::RenderScene() const
{
	myStateMachine->Render();
}

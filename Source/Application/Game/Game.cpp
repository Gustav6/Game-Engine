#include "Game.h"

#include <utility>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <memory>

#include "EngineContext.h"

#include "AssetPipeline/AssetRegistry/AssetRegistry.h"

#include "AssetPipeline/Assets/Types/MeshAsset.h"
#include "AssetPipeline/Assets/Types/AnimationAsset.h"
#include "AssetPipeline/Assets/Types/MaterialAsset.h"
#include "AssetPipeline/Assets/Types/TextureAsset.h"
#include "AssetPipeline/Assets/Types/FontAsset.h"
#include "AssetPipeline/Assets/Types/SpriteAsset.h"
#include "AssetPipeline/Assets/Types/UnrealSceneAsset.h"

#include "GameState/GameStateMachine.h"

#include "GamePipeline/World/Actor.hpp"
#include "GamePipeline/World/GameWorld.h"
#include "GamePipeline/Components/Registry/ComponentRegistryManager.h"

#include "GraphicsEngine/GraphicsEngine.hpp"
#include "GraphicsEngine/Core/RenderEnums.h"

#include "GraphicsEngine/Core/SceneCamera.h"
#include "GraphicsEngine/RHI/GraphicsCommandList.h"

#include "Managers/DebugSettingsManager.h"
#include "Managers/WindowManager.h"

#include "Camera/GameCamera.h"

#include "GameState/GameStateMachine.h"
#include "Json/JsonWrapper.h"

#include "Logger/Logger.h"

#ifdef IMGUI_ENABLE
#include <ImGui/ImGuiLayer.h>
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

		if (!writer.SetString("ContentPath", "../../Content"))
		{
			throw std::runtime_error("Failed to create settings file!");
		}
		if (!writer.SetString("ShaderPath", "../../Shaders"))
		{
			throw std::runtime_error("Failed to create settings file!");
		}

		writer.Save();
	}

	Json::Data settings = Json::Read(settingsPath);

	if (settings.HasKey("ContentPath"))
	{
		EngineContext.ContentPath = GetExecutableDirectory()/ settings.GetString("ContentPath");
	}
	else
	{
		throw std::runtime_error("EngineContext settings has no content path set!");
	}

	AssetRegistry::Get().RegisterAssetType<MeshAsset>({ ".fbx" });
	AssetRegistry::Get().RegisterAssetType<AnimationAsset>({ ".fbx" });
	AssetRegistry::Get().RegisterAssetType<MaterialAsset>({ ".json" });
	AssetRegistry::Get().RegisterAssetType<TextureAsset>({ ".dds" });
	AssetRegistry::Get().RegisterAssetType<FontAsset>({ ".dds",".json" });
	AssetRegistry::Get().RegisterAssetType<SpriteAsset>({ ".dds" });
	AssetRegistry::Get().RegisterAssetType<UnrealSceneAsset>({ ".json" });

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

	// Graphics Init
	{
		SPDLOG_INFO("Initializing Graphics Engine...");

		GraphicsEngine& GE = GraphicsEngine::Get();

		if (settings.HasKey("ShaderPath"))
		{
			std::filesystem::path shaderRoot = GetExecutableDirectory() / settings.GetString("ShaderPath");

			if (!GE.Initialize(myMainWindowHandle, shaderRoot))
			{
				throw std::runtime_error("Graphics Engine failed to initialize!");
			}
		}
		else
		{
			throw std::runtime_error("EngineContext settings has no shader path set!");
		}

		myGraphicsCommandList = std::make_unique<GraphicsCommandList>();

		GE.CreateCommandList("Application", *myGraphicsCommandList);

		std::shared_ptr<TextureAsset> environmentTexture = AssetRegistry::Get().GetAsset<TextureAsset>("T_Shipyard_cm");

		if (environmentTexture != nullptr)
		{
			GE.SetEnvironmentTexture(environmentTexture->GetTexture());
		}

#ifdef IMGUI_ENABLE
		myImGuiLayer = std::make_unique<ImGuiLayer>(myMainWindowHandle);
		GE.SetRenderOverlay(myImGuiLayer.get());
#endif
	}

	{
		GameCamera& gameCamera = GameCamera::GetInstance();
		gameCamera.Init(90.0f, 1.0f, 10000.0f, GraphicsEngine::Get().GetClientSize());
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
				.Transform = GameCamera::GetInstance().GetTransform().GetWorldMatrix(),
				.RenderCamera = GameCamera::GetInstance().GetRenderCamera()
			};

			FrameStats frameStats = GE.Render(*myGraphicsCommandList, camera);
		
#ifdef IMGUI_ENABLE
			DrawImGuiLayer(frameStats);
#endif
	
			myGraphicsCommandList->FinishCommandList();

			GE.ExecuteCommandList(*myGraphicsCommandList);
			GE.Present();
		}
	}

	// Cleanup
	AssetRegistry::Get().Shutdown();
	ComponentRegistryManager::Get().Shutdown();
	GraphicsEngine::Get().Shutdown();

	return 0;
}

void Game::UpdateScene(float aDeltaTime) const
{
	GameCamera::GetInstance().Update(aDeltaTime);

	myStateMachine->Update(aDeltaTime);
}

void Game::RenderScene() const
{
	myStateMachine->Render();
}

#ifdef IMGUI_ENABLE
void Game::DrawImGuiLayer(const FrameStats& aFrameStats)
{
	ImGui::PushFont(nullptr, 18.0f);

	ImGui::ShowDemoWindow();

	if (myShowDebugWindow)
	{
		ImGui::Begin("Debug", &myShowDebugWindow, ImGuiWindowFlags_AlwaysAutoResize);

		if (ImGui::CollapsingHeader("Frame Info"))
		{
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::Text("DrawCallCount: %u", aFrameStats.DrawCallCount);
			ImGui::Text("PipelineSwitches: %u", aFrameStats.PipelineSwitches);
			ImGui::Text("VisibleObjects: %u", aFrameStats.VisibleObjects);
			ImGui::Text("CulledObjects: %u", aFrameStats.CulledObjects);
		}

		if (ImGui::CollapsingHeader("Render Settings"))
		{
			RenderSettings settings = GraphicsEngine::Get().GetSettings();

			const char* debugRenderModes[] =
			{
				"None",
				"sRGB",
				"Roughness",
				"Metalness",
				"AO (Texture)",
				"AO (Screen Space)",
				"Normals (Tangent Space)",
				"Normals (World Space)",
				"Shadows"
			};

			int debugRenderMode = static_cast<int>(settings.DebugRenderMode);

			if (ImGui::Combo("Debug Render Mode", &debugRenderMode, debugRenderModes, IM_ARRAYSIZE(debugRenderModes)))
			{
				settings.DebugRenderMode = static_cast<DebugRenderMode>(debugRenderMode);
			}

			ImGui::Separator();

			const char* toneMappers[] =
			{
				"None",
				"ACES",
				"Lottes",
				"Unreal"
			};

			int toneMapper = static_cast<int>(settings.ToneMapper);

			if (ImGui::Combo("Tone Mapper", &toneMapper, toneMappers, IM_ARRAYSIZE(toneMappers)))
			{
				settings.ToneMapper = static_cast<ToneMapper>(toneMapper);
			}

			ImGui::Separator();

			ImGui::Checkbox("Bloom", &settings.BloomEnabled);
			ImGui::Checkbox("SSAO", &settings.SSAOEnabled);
			ImGui::Checkbox("VSync", &settings.VSyncEnabled);

			GraphicsEngine::Get().SetSettings(settings);
		}

		if (ImGui::CollapsingHeader("Debug Settings"))
		{
			DebugSettings settings = DebugSettingsManager::Get().GetSettings();

			ImGui::Checkbox("Draw Physics Colliders", &settings.ColliderDebugDrawEnabled);
			ImGui::ColorEdit4("Collider Color", settings.ColliderColor.values);

			ImGui::Separator();

			ImGui::Checkbox("Draw Mesh Bounding Box", &settings.MeshDebugDrawBoxEnabled);
			ImGui::ColorEdit4("Bounding Box Color", settings.MeshBoundingBoxColor.values);

			ImGui::Separator();

			ImGui::Checkbox("Draw Mesh Bounding Sphere", &settings.MeshDebugDrawSphereEnabled);
			ImGui::ColorEdit4("Bounding Sphere Color", settings.MeshBoundingSphereColor.values);

			ImGui::Separator();
			ImGui::Checkbox("Draw Light Gizmos", &settings.LightGizmosDebugDrawEnabled);
			ImGui::ColorEdit4("Gizmo Color", settings.LightGizmoColor.values);

			DebugSettingsManager::Get().SetSettings(settings);
		}

		ImGui::End();
	}

	if (myShowLevelSelectorWindow)
	{
		ImGui::Begin("Level Selector", &myShowLevelSelectorWindow, ImGuiWindowFlags_AlwaysAutoResize);

		std::filesystem::path sceneRoot = std::filesystem::absolute(EngineContext.ContentPath / "Scenes");

		if (std::filesystem::exists(sceneRoot))
		{
			for (const auto& item : std::filesystem::recursive_directory_iterator(sceneRoot))
			{
				if (item.is_regular_file() && item.path().has_extension())
				{
					std::string lowerCaseFileExtension = item.path().extension().string();
					std::ranges::transform(lowerCaseFileExtension, lowerCaseFileExtension.begin(), tolower);

					if (lowerCaseFileExtension == ".json")
					{
						const std::string name = item.path().stem().string();

						if (ImGui::Selectable(name.c_str()))
						{
							std::shared_ptr<UnrealSceneAsset> scene = AssetRegistry::Get().GetAsset<UnrealSceneAsset>(name);

							if (scene != nullptr)
							{
								GameWorld::Get().Clear();
								GameWorld::Get().AddActors(std::move(scene->GetSceneActors()));

								// TODO : Load nav mesh, maybe move the whole block to a scene manager class?
							}
							else
							{
								SPDLOG_ERROR("Failed to load unreal scene [{}]!", name);
							}
						}
					}
				}
			}
		}

		ImGui::End();
	}

	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_None;

		ImGui::Begin("Scene Hierarchy", nullptr, flags);

		for (auto& actor : GameWorld::Get().GetActors())
		{
			if (actor->GetParent() == nullptr)
			{
				DrawActorNode(actor.get());
			}
		}

		ImGui::End();
	}

	ImGui::PopFont();
}
#endif

#ifdef IMGUI_ENABLE
void Game::DrawActorNode(Actor* aActor)
{
	ImGuiTreeNodeFlags flags =
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_SpanAvailWidth;

	if (aActor->GetChildren().empty())
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}
	else
	{
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;
	}

	if (ImGui::TreeNodeEx(aActor, flags, "%s", aActor->GetName().c_str()))
	{
		if (ImGui::IsItemClicked())
		{
			// Open window to inspect actor
			std::cout << aActor->GetName() << std::endl;
		}

		for (const auto& child : aActor->GetChildren())
		{
			DrawActorNode(child);
		}    
		
		ImGui::TreePop();
	}
}
#endif

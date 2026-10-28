#include "Game.h"

#include <utility>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <memory>

#include "Core/EngineContext.h"

#include "Core/Assets/Registry/AssetRegistry.h"

#include "Core/Assets/Types/MeshAsset.h"
#include "Core/Assets/Types/AnimationAsset.h"
#include "Core/Assets/Types/MaterialAsset.h"
#include "Core/Assets/Types/TextureAsset.h"
#include "Core/Assets/Types/FontAsset.h"
#include "Core/Assets/Types/SpriteAsset.h"
#include "Core/Assets/Types/UnrealSceneAsset.h"

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
#include <Editor/ImGuiLayer.h>
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

	// Graphics Initialize
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

#ifndef _RETAIL
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

#ifndef _RETAIL
			DrawImGuiLayer(frameStats);
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
	GameCamera::GetInstance().Update(aDeltaTime);

	myStateMachine->Update(aDeltaTime);
}

void Game::RenderScene() const
{
	myStateMachine->Render();
}

#ifndef _RETAIL
void Game::DrawImGuiLayer(const FrameStats& aFrameStats)
{
	ImGui::PushFont(nullptr, 18.0f);

	//ImGui::ShowDemoWindow();

	if (myShowDebugWindow)
	{
		ImGui::Begin("Debug", &myShowDebugWindow, ImGuiWindowFlags_AlwaysAutoResize);

		if (ImGui::CollapsingHeader("Frame Info"))
		{
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::Text("Draw Call Count: %u", aFrameStats.DrawCallCount);
			ImGui::Text("Pipeline Switches: %u", aFrameStats.PipelineSwitches);
			ImGui::Text("Visible Objects: %u", aFrameStats.VisibleObjects);
			ImGui::Text("Culled Objects: %u", aFrameStats.CulledObjects);
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

			ImGui::Text("Debug Render Mode");

			if (ImGui::Combo("##DebugRenderMode", &debugRenderMode, debugRenderModes, IM_ARRAYSIZE(debugRenderModes)))
			{
				settings.DebugRenderMode = static_cast<DebugRenderMode>(debugRenderMode);
			}

			ImGui::Separator();

			const char* toneMappers[] =
			{
				"None",
				"ACES",
				"Lottes",
				"Unreal",
				"AGX",
				"AGXPunchy",
				"Uncharted2",
				"Uchimura",
				"Filmic",
				"Neutral",
				"FilmicHejl"
			};

			int toneMapper = static_cast<int>(settings.ToneMapper);

			ImGui::Text("Tone Mapper");

			if (ImGui::Combo("##ToneMapper", &toneMapper, toneMappers, IM_ARRAYSIZE(toneMappers)))
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

			{
				ImGui::Text("Physics Collider");

				ImGui::Checkbox("##PhysicsCollider", &settings.ColliderDebugDrawEnabled);

				ImGui::SameLine();

				float* color = settings.ColliderColor.values;

				if (ImGui::ColorButton("Collider Color Button", ImVec4(color[0], color[1], color[2], color[3])))
				{
					ImGui::OpenPopup("Physics Color");
				}

				if (ImGui::BeginPopup("Physics Color"))
				{
					ImGui::ColorPicker4("##picker", color);
					ImGui::EndPopup();
				}

				ImGui::Separator();
			}

			{
				ImGui::Text("Bounding Box Collider");

				ImGui::Checkbox("##BoundingBoxCollider", &settings.MeshDebugDrawBoxEnabled);

				ImGui::SameLine();

				float* color = settings.MeshBoundingBoxColor.values;

				if (ImGui::ColorButton("Bounding Box Color Button", ImVec4(color[0], color[1], color[2], color[3])))
				{
					ImGui::OpenPopup("Mesh Box Popup");
				}

				if (ImGui::BeginPopup("Mesh Box Popup"))
				{
					ImGui::ColorPicker4("##picker", color);
					ImGui::EndPopup();
				}

				ImGui::Separator();
			}

			{
				ImGui::Text("Bounding Sphere Collider");

				ImGui::Checkbox("##BoundingSphereCollider", &settings.MeshDebugDrawSphereEnabled);

				ImGui::SameLine();

				float* color = settings.MeshBoundingSphereColor.values;

				if (ImGui::ColorButton("Bounding Sphere Color Button", ImVec4(color[0], color[1], color[2], color[3])))
				{
					ImGui::OpenPopup("Mesh Sphere Popup");
				}

				if (ImGui::BeginPopup("Mesh Sphere Popup"))
				{
					ImGui::ColorPicker4("##picker", color);
					ImGui::EndPopup();
				}

				ImGui::Separator();
			}

			{
				ImGui::Text("Light Gizmo Collider");

				ImGui::Checkbox("##LightGizmoCollider", &settings.LightGizmosDebugDrawEnabled);

				ImGui::SameLine();

				float* color = settings.LightGizmoColor.values;

				if (ImGui::ColorButton("Light Gizmo Color Button", ImVec4(color[0], color[1], color[2], color[3])))
				{
					ImGui::OpenPopup("Light Gizmo Color");
				}

				if (ImGui::BeginPopup("Light Gizmo Color"))
				{
					ImGui::ColorPicker4("##picker", color);
					ImGui::EndPopup();
				}
			}

			DebugSettingsManager::Get().SetSettings(settings);
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

#ifndef _RETAIL
void Game::DrawActorNode(Actor* aActor)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DrawLinesToNodes;

	if (aActor->GetChildren().empty())
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}
	else
	{
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;
	}

	if (mySelectedActor == aActor)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	bool isOpen = ImGui::TreeNodeEx(aActor, flags, "%s", aActor->GetName().c_str());

	if (ImGui::IsItemClicked() && mySelectedActor != aActor)
	{
		mySelectedActor = aActor;

		std::cout << mySelectedActor->GetName() << ", Selected" << "\n";

		PrintNodesChildren(mySelectedActor);
	}

	if (isOpen)
	{
		for (const auto& child : aActor->GetChildren())
		{
			DrawActorNode(child);
		}

		ImGui::TreePop();
	}
}

void Game::PrintNodesChildren(Actor* aActor)
{
	for (const auto& child : aActor->GetChildren())
	{
		std::cout << child->GetName() << ", Owned by - " << aActor->GetName() << "\n";

		PrintNodesChildren(child);
	}
}
#endif

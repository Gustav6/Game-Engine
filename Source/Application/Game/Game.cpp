#include "Game.h"

#include <utility>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>
#include <memory>

#include "Application.h"

#include "AssetHandling/Assets/Types/MeshAsset.h"
#include "AssetHandling/Assets/Types/AnimationAsset.h"
#include "AssetHandling/Assets/Types/MaterialAsset.h"
#include "AssetHandling/Assets/Types/TextureAsset.h"
#include "AssetHandling/Assets/Types/UnrealSceneAsset.h"

#include "Game/Components/Rendering/MeshComponent.h"
#include "Game/Components/Rendering/AnimationComponent.h"
#include "Game/Components/Light/DirectionlLightComponent.h"
#include "Game/Components/Light/PointLightComponent.h"
#include "Game/Components/Light/SpotLightComponent.h"

#include "GraphicsEngine/GraphicsEngine.hpp"
#include "GraphicsEngine/Objects/Texture.h"
#include "GraphicsEngine/RHI/GraphicsCommandList.h"

#include "tracy/core/Tracy.hpp"
#include <Audio/FmodAudioSystem.h>

Game::Game() = default;
Game::~Game() = default;

bool Game::Initialize(SIZE aWindowSize, WNDPROC aWindowProcess, LPCWSTR aWindowTitle)
{
    myScene = std::make_unique<Scene>();

    std::filesystem::path workingDirectory = GetExecutableDirectory().parent_path().parent_path() / "Content";

    Application.AudioSystem = std::make_unique<FmodAudioSystem>();

    if (!Application.AudioSystem->Initialize()) 
    {
        MVLOG(Error, "Failed to initialize audio system.");
    }

    Application.AssetRegistry.RegisterAssetType<MeshAsset>({ ".fbx" });
    Application.AssetRegistry.RegisterAssetType<AnimationAsset>({ ".fbx" });
    Application.AssetRegistry.RegisterAssetType<MaterialAsset>({ ".json" });
    Application.AssetRegistry.RegisterAssetType<TextureAsset>({ ".dds" });
    Application.AssetRegistry.RegisterAssetType<UnrealSceneAsset>({ ".json" });

    if (!Application.AssetRegistry.Initialize(workingDirectory))
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

    MVLOG(Log, "Ready!");

    // Graphics Init
	{
        MVLOG(Log, "Initializing Graphics Engine...");

		GraphicsEngine& GE = GraphicsEngine::Get();

	    if(!GE.Initialize(myMainWindowHandle, workingDirectory / "Shaders"))
	    {
            return false;
        }

		std::unique_ptr<Texture> environmentTexture = std::make_unique<Texture>();

		GE.LoadTexture(workingDirectory / "Textures" / "Environment Map" / "T_Shipyard.dds", *environmentTexture);
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
            }
        }

        Application.TimeManager.Update();
        Application.InputManager.UpdateInput();

        UpdateScene(Application.TimeManager.GetDeltaTime());

        Application.AudioSystem->Update();
        BuildSceneForRender();

		// Render to graphics engine and present to screen
        {
            GraphicsEngine& GE = GraphicsEngine::Get();

            myGraphicsCommandList->ResetCommandList();

            GE.Render(*myGraphicsCommandList, *myScene);

            myGraphicsCommandList->FinishCommandList();

            GE.ExecuteCommandList(*myGraphicsCommandList);
            GE.Present();
        }
    }

    // Cleanup
    Application.AssetRegistry.UnInitialize();

    if (Application.AudioSystem)
    {
        Application.AudioSystem->Shutdown();
        Application.AudioSystem.reset();
    }

    return 0;
}

void Game::LoadScene()
{
    {
        myGameCamera = GameCamera(90.0f, 1.0f, 10000.0f, GraphicsEngine::Get().GetClientSize());
        myGameCamera.GetTransform().SetPosition({ 0, 100, -200 });
    }

    {
        std::unique_ptr<Actor> object = std::make_unique<Actor>();

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetRegistry.GetAsset<MeshAsset>("SM_Color_Checker"));
        meshComponent->SetMaterial(0, Application.AssetRegistry.GetAsset<MaterialAsset>("SMColorChecker"));

        object->GetTransform().Translate({ -400, 40, 0 });
        object->GetTransform().Rotate(CommonUtilities::Vector3f{ -90, 0, 0 });

        myGameWorld.AddActor(std::move(object));
    }

    {
        std::unique_ptr<Actor> object = std::make_unique<Actor>();

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetRegistry.GetAsset<MeshAsset>("SM_Chest"));
        meshComponent->SetMaterial(0, Application.AssetRegistry.GetAsset<MaterialAsset>("ChestMaterial"));
		
        object->GetTransform().Rotate(CommonUtilities::Vector3f{ 0, -180, 0 });
        object->GetTransform().Translate({ 200, 0, 400 });

        myGameWorld.AddActor(std::move(object));
    }

    {
        std::unique_ptr<Actor> object = std::make_unique<Actor>();

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetRegistry.GetAsset<MeshAsset>("Unit_Plane"));
        meshComponent->SetMaterial(0, Application.AssetRegistry.GetAsset<MaterialAsset>("BaseMaterial"));

        object->GetTransform().SetScale({ 2000, 2000, 2000 });

        myGameWorld.AddActor(std::move(object));
    }

    {
        std::unique_ptr<Actor> object = std::make_unique<Actor>();

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetRegistry.GetAsset<MeshAsset>("Unit_Cube"));
        meshComponent->SetMaterial(0, Application.AssetRegistry.GetAsset<MaterialAsset>("BaseMaterial"));

        object->GetTransform().Translate({ 0, 50, 0 });
        object->GetTransform().SetScale({ 100, 100, 100 });

        myGameWorld.AddActor(std::move(object));
    }

    {
        std::unique_ptr<Actor> object = std::make_unique<Actor>();

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetRegistry.GetAsset<MeshAsset>("Unit_Cube"));
        meshComponent->SetMaterial(0, Application.AssetRegistry.GetAsset<MaterialAsset>("BaseMaterial"));

        object->GetTransform().SetEulerRotation({ 45.0f, 180.0f, 45.0f });
        object->GetTransform().Translate({ -200, 100, 400 });
        object->GetTransform().SetScale({ 100, 100, 100 });

        myGameWorld.AddActor(std::move(object));
    }

    {
        std::unique_ptr<Actor> object = std::make_unique<Actor>();

        object->GetTransform().Translate({ 200, 0, 0 });

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetRegistry.GetAsset<MeshAsset>("SK_C_TGA_Bro"));
        meshComponent->SetMaterial(0, Application.AssetRegistry.GetAsset<MaterialAsset>("TGABroMaterial"));

        AnimationComponent* animationComponent = object->AddComponent<AnimationComponent>(meshComponent->GetSkeleton());

        AnimationEntry breathingAnimation =
        {
            .AnimationAsset = Application.AssetRegistry.GetAsset<AnimationAsset>("A_C_TGA_Bro_Idle_Breathing"),
            .ShouldLoop = true
        };
        AnimationEntry waveAnimation =
        {
            .AnimationAsset = Application.AssetRegistry.GetAsset<AnimationAsset>("A_C_TGA_Bro_Idle_Wave"),
            .ShouldLoop = true
        };
        AnimationEntry runAnimation =
        {
            .AnimationAsset = Application.AssetRegistry.GetAsset<AnimationAsset>("A_C_TGA_Bro_Run"),
            .ShouldLoop = true
        };
        AnimationEntry walkAnimation =
        {
            .AnimationAsset = Application.AssetRegistry.GetAsset<AnimationAsset>("A_C_TGA_Bro_Walk"),
            .ShouldLoop = true
        };

        animationComponent->AddAnimation("Breathing", breathingAnimation);
        animationComponent->AddAnimation("Wave", waveAnimation);
        animationComponent->AddAnimation("Run", runAnimation);
        animationComponent->AddAnimation("Walk", walkAnimation);
        
        myAnimatedObject = myGameWorld.AddActor(std::move(object));
    }

    {
        std::unique_ptr<Actor> object = std::make_unique<Actor>();

        DirectionalLightComponent* directionalLight = object->AddComponent<DirectionalLightComponent>();

        directionalLight->SetColor({ 1.0f, 0.95f, 0.85f });
        directionalLight->SetIntensity(1.0f);
        directionalLight->SetDirection({ 0.0f, -0.75f, 1.0f });

        myDirectionLightObject = myGameWorld.AddActor(std::move(object));
    }

    {
        std::unique_ptr<Actor> object = std::make_unique<Actor>();

        object->SetEnabled(false);

        PointLightComponent* pointLight = object->AddComponent<PointLightComponent>();
        pointLight->SetPosition({ -200.0f, 150.0f, 0.0f });
        pointLight->SetColor({ 0.0f, 0.0f, 1.0f });
        pointLight->SetIntensity(75000.0f);
        pointLight->SetRadius(900.0f);
        pointLight->SetShadowMapSize({ 512, 512 });

        myPointLightObject = myGameWorld.AddActor(std::move(object));
    }

    {
        std::unique_ptr<Actor> object = std::make_unique<Actor>();

        object->SetEnabled(false);

        SpotLightComponent* spotLight = object->AddComponent<SpotLightComponent>();
        spotLight->SetPosition({ 0.0f, 50.0f, -300.0f });
        spotLight->SetColor({ 0.0f, 1.0f, 0.0f });
        spotLight->SetIntensity(50000.0f);
        spotLight->SetRadius(1000.0f);
        spotLight->SetDirection({ 0.0f, 0.0f, 1.0f });
        spotLight->SetCone(0.0f, 0.7f);
        spotLight->SetShadowMapSize({ 512, 512 });

        mySpotLightObject = myGameWorld.AddActor(std::move(object));
    }
}

void Game::UpdateScene(float aDeltaTime)
{
    ZoneScopedN("Update Game");

    myGameWorld.Update(aDeltaTime);

    // Camera controls
    {
        if (Application.InputManager.IsKeyDown(KeyCode::W))
        {
            CommonUtilities::Vector3f delta = myGameCamera.GetTransform().GetForward() * myCamerasMoveSpeed * aDeltaTime;

            myGameCamera.GetTransform().Translate(delta);
            myGameCamera.UpdateFrustum();
        }
        if (Application.InputManager.IsKeyDown(KeyCode::A))
        {
            CommonUtilities::Vector3f delta = -myGameCamera.GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime;

            myGameCamera.GetTransform().Translate(delta);
            myGameCamera.UpdateFrustum();
        }
        if (Application.InputManager.IsKeyDown(KeyCode::S))
        {
            CommonUtilities::Vector3f delta = -myGameCamera.GetTransform().GetForward() * myCamerasMoveSpeed * aDeltaTime;

            myGameCamera.GetTransform().Translate(delta);
            myGameCamera.UpdateFrustum();
        }
        if (Application.InputManager.IsKeyDown(KeyCode::D))
        {
            CommonUtilities::Vector3f delta = myGameCamera.GetTransform().GetRight() * myCamerasMoveSpeed * aDeltaTime;

            myGameCamera.GetTransform().Translate(delta);
            myGameCamera.UpdateFrustum();
        }

        if (Application.InputManager.IsKeyDown(KeyCode::Space))
        {
            CommonUtilities::Vector3f delta = myGameCamera.GetTransform().GetUp() * myCamerasMoveSpeed * aDeltaTime;

            myGameCamera.GetTransform().Translate(delta);
            myGameCamera.UpdateFrustum();
        }
        if (Application.InputManager.IsKeyDown(KeyCode::Control))
        {
            CommonUtilities::Vector3f delta = -myGameCamera.GetTransform().GetUp() * myCamerasMoveSpeed * aDeltaTime;

            myGameCamera.GetTransform().Translate(delta);
            myGameCamera.UpdateFrustum();
        }

        if (Application.InputManager.IsKeyDown(KeyCode::RMB))
        {
            const float sensitivity = 0.2f;

            CommonUtilities::Vector2f mouseDelta = Application.InputManager.GetMousePositionDelta();

            float yaw = mouseDelta.x * sensitivity;
            float pitch = mouseDelta.y * sensitivity;

            CommonUtilities::Quaternion<float> pitchQuat({ 1, 0, 0 }, pitch);
            CommonUtilities::Quaternion<float> yawQuat({ 0, 1, 0 }, yaw);

            myGameCamera.GetTransform().Rotate(pitchQuat, false);
            myGameCamera.GetTransform().Rotate(yawQuat, true);
            myGameCamera.UpdateFrustum();
        }

		if (Application.InputManager.GetScrollWheelDelta() > 0) 
		{
			myCamerasMoveSpeed *= 1.05f;
		}
		else if (Application.InputManager.GetScrollWheelDelta() < 0) 
		{
			myCamerasMoveSpeed *= 0.95f;
		}

        if (Application.InputManager.IsKeyPressed(KeyCode::RMB))
        {
            Application.InputManager.CaptureMouse();
            Application.InputManager.HideMouse();
        }
        else if (Application.InputManager.IsKeyReleased(KeyCode::RMB))
        {
            Application.InputManager.ReleaseMouse();
            Application.InputManager.ShowMouse();
        }
    }

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

	if (Application.InputManager.IsKeyPressed(KeyCode::F6))
	{
		GraphicsEngine::Get().CycleToNextRenderPass();
	}

    if (Application.InputManager.IsKeyReleased(KeyCode::Escape))
    {
        PostQuitMessage(0);
    }
}

void Game::BuildSceneForRender()
{
    ZoneScopedN("Render Scene");

    if (myScene == nullptr)
    {
        return;
    }

    myScene->SceneLights.clear();
    myScene->DrawCalls.clear();

    for (auto& actor : myGameWorld.GetActors())
    {
        if (!actor->GetIsEnabled())
        {
            continue;
        }

        {
            MeshComponent* meshComponent = actor.get()->GetComponent<MeshComponent>();

            if (meshComponent != nullptr)
            {
                DrawCall drawCall = 
                {
                    .Mesh = &meshComponent->GetMesh(),
                    .Transform = actor->GetTransform().GetWorldMatrix()
                };

                const auto& materialAssets = meshComponent->GetMaterialAssetList();

                for (size_t i = 0; i < materialAssets.size(); i++)
                {
                    drawCall.Materials[i] = materialAssets[i]->GetMaterial().get();
                }

                AnimationComponent* animationComponent = actor.get()->GetComponent<AnimationComponent>();

                if (animationComponent != nullptr)
                {
                    drawCall.HasSkinning = true;
                    drawCall.JointTransforms = animationComponent->GetJointTransforms();
                }

                myScene->DrawCalls.emplace_back(std::move(drawCall));
            }
        }

        {
            DirectionalLightComponent* directionalLight = actor.get()->GetComponent<DirectionalLightComponent>();

            if (directionalLight != nullptr)
            {
                directionalLight->UpdateLightViewProjection();

                SceneLight light = 
                {
                    .LightData = directionalLight->GetData(),
                    .Transform = directionalLight->GetTransform(),
                    .Projection = directionalLight->GetProjection()
                };

                myScene->SceneLights.emplace_back(light);
            }
        }

        {
            PointLightComponent* pointLight = actor.get()->GetComponent<PointLightComponent>();

            if (pointLight != nullptr)
            {
                SceneLight light =
                {
                    .LightData = pointLight->GetData(),
                    .ShadowMap = pointLight->GetShadowMap(),
                    .Transform = pointLight->GetTransform(),
                    .Projection = pointLight->GetProjection()
                };

                myScene->SceneLights.emplace_back(light);
            }
        }

        {
            SpotLightComponent* spotLight = actor.get()->GetComponent<SpotLightComponent>();

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

                myScene->SceneLights.emplace_back(light);
            }
        }
    }

    myScene->SceneCamera = 
	{ 
		.Transform = myGameCamera.GetTransform().GetWorldMatrix(), 
		.RenderCamera = myGameCamera.GetCamera()
	};
}

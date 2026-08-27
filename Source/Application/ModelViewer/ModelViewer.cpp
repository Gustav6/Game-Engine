#include "ModelViewer.h"

#include <algorithm>
#include <filesystem>
#include <future>
#include <iostream>
#include <vector>

#include "Application.h"

#include "Game/Component/Rendering/MeshComponent.h"
#include "Game/Component/Rendering/AnimationComponent.h"
#include "Game/Component/Light/DirectionlLightComponent.h"
#include "Game/Component/Light/PointLightComponent.h"
#include "Game/Component/Light/SpotLightComponent.h"

#include "GraphicsEngine/GraphicsEngine.hpp"
#include "GraphicsEngine/RHI/GraphicsCommandList.h"

#include "tracy/core/Tracy.hpp"

ModelViewer::ModelViewer() = default;
ModelViewer::~ModelViewer() = default;

bool ModelViewer::Initialize(SIZE aWindowSize, WNDPROC aWindowProcess, LPCWSTR aWindowTitle)
{
    std::filesystem::path contentPath = std::filesystem::current_path().parent_path().parent_path() / "Content";

    Application.AssetLibrary.Initialize(contentPath);
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

	    if(!GE.Initialize(myMainWindowHandle, contentPath / "Shaders"))
	    {
            return false;
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

int ModelViewer::Run()
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
        BuildRenderScene();

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
    Application.AssetLibrary.UnInitialize();

    return 0;
}

void ModelViewer::LoadScene()
{
	myScene = std::make_unique<Scene>();

    {
        Application.AssetLibrary.LoadMesh("Meshes/SM_Chest.fbx");
        Application.AssetLibrary.LoadMesh("Meshes/SK_C_TGA_Bro.fbx");
        Application.AssetLibrary.LoadMesh("Meshes/SM_Color_Checker.fbx");
    }

    {
        Application.AssetLibrary.LoadAnimation("Animations/Idle/A_C_TGA_Bro_Idle_Breathing.fbx");
        Application.AssetLibrary.LoadAnimation("Animations/Idle/A_C_TGA_Bro_Idle_Wave.fbx");
        Application.AssetLibrary.LoadAnimation("Animations/Locomotion/A_C_TGA_Bro_Run.fbx");
        Application.AssetLibrary.LoadAnimation("Animations/Locomotion/A_C_TGA_Bro_Walk.fbx");
    }

    {
        Application.AssetLibrary.LoadMaterial("Materials/BaseMaterial.json");
        Application.AssetLibrary.LoadMaterial("Materials/ChestMaterial.json");
        Application.AssetLibrary.LoadMaterial("Materials/TGABroMaterial.json");
        Application.AssetLibrary.LoadMaterial("Materials/SMColorChecker.json");
    }

    {
        myGameCamera = GameCamera(90.0f, 1.0f, 5000.0f, GraphicsEngine::Get().GetClientSize());
        myGameCamera.GetTransform().SetPosition({ 0, 100, -200 });
    }

    {
        std::unique_ptr<GameObject> object = std::make_unique<GameObject>();

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetLibrary.GetMesh("Meshes/SM_Color_Checker.fbx"));
        meshComponent->SetMaterial(0, Application.AssetLibrary.GetMaterial("Materials/SMColorChecker.json"));

        object->GetTransform().Translate({ -400, 40, 0 });
        object->GetTransform().Rotate(CommonUtilities::Vector3f{ -90, 0, 0 });

        myGameWorld.AddGameObject(std::move(object));
    }

    {
        std::unique_ptr<GameObject> object = std::make_unique<GameObject>();

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetLibrary.GetMesh("Meshes/SM_Chest.fbx"));
        meshComponent->SetMaterial(0, Application.AssetLibrary.GetMaterial("Materials/ChestMaterial.json"));

        object->GetTransform().Translate({ 200, 0, 400 });

        myGameWorld.AddGameObject(std::move(object));
    }

    {
        std::unique_ptr<GameObject> object = std::make_unique<GameObject>();

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetLibrary.GetMesh("UnitPlane"));
        meshComponent->SetMaterial(0, Application.AssetLibrary.GetMaterial("Materials/BaseMaterial.json"));

        object->GetTransform().Translate({ 0, 0, 0 });
        object->GetTransform().SetScale({ 2000, 2000, 2000 });

        myGameWorld.AddGameObject(std::move(object));
    }

    {
        std::unique_ptr<GameObject> object = std::make_unique<GameObject>();

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetLibrary.GetMesh("UnitCube"));
        meshComponent->SetMaterial(0, Application.AssetLibrary.GetMaterial("Materials/BaseMaterial.json"));

        object->GetTransform().Translate({ 0, 50, 0 });
        object->GetTransform().SetScale({ 100, 100, 100 });

        myGameWorld.AddGameObject(std::move(object));
    }

    {
        std::unique_ptr<GameObject> object = std::make_unique<GameObject>();

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetLibrary.GetMesh("UnitCube"));
        meshComponent->SetMaterial(0, Application.AssetLibrary.GetMaterial("Materials/BaseMaterial.json"));

        object->GetTransform().SetEulerRotation({ 45.0f, 180.0f, 45.0f });
        object->GetTransform().Translate({ -200, 100, 400 });
        object->GetTransform().SetScale({ 100, 100, 100 });

        myGameWorld.AddGameObject(std::move(object));
    }

    {
        std::unique_ptr<GameObject> object = std::make_unique<GameObject>();

        object->GetTransform().Translate({ 200, 0, 0 });

        MeshComponent* meshComponent = object->AddComponent<MeshComponent>();

        meshComponent->SetMesh(Application.AssetLibrary.GetMesh("Meshes/SK_C_TGA_Bro.fbx"));
        meshComponent->SetMaterial(0, Application.AssetLibrary.GetMaterial("Materials/TGABroMaterial.json"));

        AnimationComponent* animationComponent = object->AddComponent<AnimationComponent>(meshComponent->GetSkeleton());

        AnimationEntry breathingAnimation =
        {
            .Animation = Application.AssetLibrary.GetAnimation("Animations/Idle/A_C_TGA_Bro_Idle_Breathing.fbx"),
            .ShouldLoop = true
        };
        AnimationEntry waveAnimation =
        {
            .Animation = Application.AssetLibrary.GetAnimation("Animations/Idle/A_C_TGA_Bro_Idle_Wave.fbx"),
            .ShouldLoop = true
        };
        AnimationEntry runAnimation =
        {
            .Animation = Application.AssetLibrary.GetAnimation("Animations/Locomotion/A_C_TGA_Bro_Run.fbx"),
            .ShouldLoop = true
        };
        AnimationEntry walkAnimation =
        {
            .Animation = Application.AssetLibrary.GetAnimation("Animations/Locomotion/A_C_TGA_Bro_Walk.fbx"),
            .ShouldLoop = true
        };

        animationComponent->AddAnimation("Breathing", breathingAnimation);
        animationComponent->AddAnimation("Wave", waveAnimation);
        animationComponent->AddAnimation("Run", runAnimation);
        animationComponent->AddAnimation("Walk", walkAnimation);
        
        myAnimatedObject = myGameWorld.AddGameObject(std::move(object));
    }

    {
        std::unique_ptr<GameObject> object = std::make_unique<GameObject>();

        DirectionalLightComponent* directionalLight = object->AddComponent<DirectionalLightComponent>();

        directionalLight->SetColor({ 1.0f, 0.95f, 0.85f });
        directionalLight->SetIntensity(2.0f);
        directionalLight->SetDirection({ 0.0f, -1.0f, 1.0f });

        myDirectionLightObject = myGameWorld.AddGameObject(std::move(object));
    }

    {
        std::unique_ptr<GameObject> object = std::make_unique<GameObject>();

        object->SetEnabled(false);

        PointLightComponent* pointLight = object->AddComponent<PointLightComponent>();
        pointLight->SetPosition({ -200.0f, 150.0f, 0.0f });
        pointLight->SetColor({ 1.0f, 0.95f, 0.85f });
        pointLight->SetIntensity(7500.0f);
        pointLight->SetRadius(900.0f);
        pointLight->SetShadowMapSize({ 512, 512 });

        myPointLightObject = myGameWorld.AddGameObject(std::move(object));
    }

    {
        std::unique_ptr<GameObject> object = std::make_unique<GameObject>();

        object->SetEnabled(false);

        SpotLightComponent* spotLight = object->AddComponent<SpotLightComponent>();
        spotLight->SetPosition({ 0.0f, 50.0f, -200.0f });
        spotLight->SetColor({ 1.0f, 0.95f, 0.85f });
        spotLight->SetIntensity(30000.0f);
        spotLight->SetRadius(1000.0f);
        spotLight->SetDirection({ 0.0f, 0.0f, 1.0f });
        spotLight->SetCone(0.0f, 0.7f);
        spotLight->SetShadowMapSize({ 512, 512 });

        mySpotLightObject = myGameWorld.AddGameObject(std::move(object));
    }
}

void ModelViewer::UpdateScene(float aDeltaTime)
{
    ZoneScopedN("Update Game");

    myGameWorld.Update(aDeltaTime);

    // Camera controls
    {
        const float moveSpeed = 400.0f;

        if (Application.InputManager.IsKeyDown(KeyCode::W))
        {
            CommonUtilities::Vector3f delta = myGameCamera.GetTransform().GetForward() * moveSpeed * aDeltaTime;

            myGameCamera.GetTransform().Translate(delta);
            myGameCamera.UpdateFrustum();
        }
        if (Application.InputManager.IsKeyDown(KeyCode::A))
        {
            CommonUtilities::Vector3f delta = -myGameCamera.GetTransform().GetRight() * moveSpeed * aDeltaTime;

            myGameCamera.GetTransform().Translate(delta);
            myGameCamera.UpdateFrustum();
        }
        if (Application.InputManager.IsKeyDown(KeyCode::S))
        {
            CommonUtilities::Vector3f delta = -myGameCamera.GetTransform().GetForward() * moveSpeed * aDeltaTime;

            myGameCamera.GetTransform().Translate(delta);
            myGameCamera.UpdateFrustum();
        }
        if (Application.InputManager.IsKeyDown(KeyCode::D))
        {
            CommonUtilities::Vector3f delta = myGameCamera.GetTransform().GetRight() * moveSpeed * aDeltaTime;

            myGameCamera.GetTransform().Translate(delta);
            myGameCamera.UpdateFrustum();
        }

        if (Application.InputManager.IsKeyDown(KeyCode::Space))
        {
            CommonUtilities::Vector3f delta = myGameCamera.GetTransform().GetUp() * moveSpeed * aDeltaTime;

            myGameCamera.GetTransform().Translate(delta);
            myGameCamera.UpdateFrustum();
        }
        if (Application.InputManager.IsKeyDown(KeyCode::Control))
        {
            CommonUtilities::Vector3f delta = -myGameCamera.GetTransform().GetUp() * moveSpeed * aDeltaTime;

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
    if (Application.InputManager.IsKeyReleased(KeyCode::Escape))
    {
        PostQuitMessage(0);
    }
}

void ModelViewer::BuildRenderScene()
{
    ZoneScopedN("Render Scene");

    myScene->SceneLights.clear();
    myScene->DrawCalls.clear();

    for (auto& gameObject : myGameWorld.GetGameObjects())
    {
        if (!gameObject->GetIsEnabled())
        {
            continue;
        }

        {
            MeshComponent* meshComponent = gameObject.get()->GetComponent<MeshComponent>();

            if (meshComponent != nullptr)
            {
                DrawCall drawCall = 
                {
                    .Mesh = meshComponent->GetMesh().get(),
                    .Transform = gameObject->GetTransform().GetWorldMatrix()
                };

                const auto& materials = meshComponent->GetMaterialList();

                for (size_t i = 0; i < materials.size(); i++)
                {
                    drawCall.Materials[i] = materials[i].get();
                }

                AnimationComponent* animationComponent = gameObject.get()->GetComponent<AnimationComponent>();

                if (animationComponent != nullptr)
                {
                    drawCall.HasSkinning = true;
                    drawCall.JointTransforms = animationComponent->GetJointTransforms();
                }

                myScene->DrawCalls.emplace_back(std::move(drawCall));
            }
        }

        {
            DirectionalLightComponent* directionalLight = gameObject.get()->GetComponent<DirectionalLightComponent>();

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
            PointLightComponent* pointLight = gameObject.get()->GetComponent<PointLightComponent>();

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
            SpotLightComponent* spotLight = gameObject.get()->GetComponent<SpotLightComponent>();

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
		.Camera = myGameCamera.GetCamera()
	};
}

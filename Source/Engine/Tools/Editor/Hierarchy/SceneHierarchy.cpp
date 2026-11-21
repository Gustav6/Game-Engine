#include "Tools/Tools.pch.h"
#include "SceneHierarchy.h"

#include <ImGui/imgui.h>

#include <Wrappers/Logger/LoggerWrapper.h>

#include <Core/Scene/World/Actor.hpp>
#include <Core/Scene/World/GameWorld.h>

#include <Graphics/GraphicsEngine.hpp>

#include <Graphics/Core/SceneCamera.h>
#include <Graphics/Objects/Scene/Camera.h>
#include <Graphics/Drawers/DebugDrawer.h>

#include <Core/EngineContext.h>
#include <Core/Scene/Components/Concrete/Transform/Transform.h>
#include <Core/Scene/Components/Concrete/Rendering/MeshComponent.h>
#include <Core/Scene/Components/Concrete/Rendering/InstancedMeshComponent.h>

#include <ImGuizmo/ImGuizmo.h>

void SceneHierarchy::Update(float aDeltaTime)
{
	aDeltaTime;

	if (!ImGuizmo::IsOver() && !ImGuizmo::IsUsing())
	{
		if (GEngineContext.InputManager.IsKeyPressed(KeyCode::LMB))
		{
			CommonUtilities::Vector2f mousePosition = GEngineContext.InputManager.GetMousePosition();

			unsigned int mouseX = static_cast<unsigned int>(mousePosition.x);
			unsigned int mouseY = static_cast<unsigned int>(mousePosition.y);

			GraphicsEngine::Get().RequestPixelPick(mouseX, mouseY);
		}
	}

	if (GEngineContext.InputManager.IsKeyPressed(KeyCode::W))
	{
		myCurrentOperation = Operation::Translate;
	}
	if (GEngineContext.InputManager.IsKeyPressed(KeyCode::E))
	{
		myCurrentOperation = Operation::Rotate;
	}
	if (GEngineContext.InputManager.IsKeyPressed(KeyCode::R))
	{
		myCurrentOperation = Operation::Scale;
	}

	if (GEngineContext.InputManager.IsKeyPressed(KeyCode::F))
	{
		GEngineContext.GameCamera->FocusOn(mySelectedActor);
	}

	if (GEngineContext.InputManager.IsKeyPressed(KeyCode::Delete))
	{
		if (mySelectedActor != nullptr)
		{
			mySelectedActor->Destroy();
			mySelectedActor = nullptr;
		}
	}

	if (GEngineContext.InputManager.IsKeyPressed(KeyCode::Escape))
	{
		SetSelectedActor(nullptr);
	}
}

void SceneHierarchy::Render()
{
	if (mySelectedActor != nullptr)
	{
		MeshComponent* mesh = mySelectedActor->GetComponent<MeshComponent>();

		if (mesh != nullptr)
		{
			mesh->RenderOutline();
		}

		InstancedMeshComponent* instancedMesh = mySelectedActor->GetComponent<InstancedMeshComponent>();

		if (instancedMesh != nullptr)
		{
			instancedMesh->RenderOutline();
		}
	}

	ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DrawLinesToNodes | ImGuiTreeNodeFlags_DefaultOpen;
	bool isOpen = ImGui::TreeNodeEx("Scene", flags, "%s", "Scene");
	
	if (isOpen)
	{
		for (auto& actor : GameWorld::Get().GetActors())
		{
			if (actor->GetParent() == nullptr)
			{
				DisplayActorNode(actor);
			}
		}

		ImGui::TreePop();
	}

	ImGui::End();

	if (myCameraReference != nullptr && mySelectedActor != nullptr)
	{
		ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());

		ImVec2 viewportPos = ImGui::GetMainViewport()->Pos;
		ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

		ImGuizmo::SetRect(
			viewportPos.x,
			viewportPos.y,
			viewportSize.x,
			viewportSize.y
		);

		Transform* transform = mySelectedActor->GetComponent<Transform>();

		float* matrix = transform->GetWorldMatrix().GetData();

		ImGuizmo::MODE mode = ImGuizmo::WORLD;

		switch (myCurrentMode)
		{
			case SceneHierarchy::Mode::Local:
			{
				mode = ImGuizmo::LOCAL;
				break;
			}
			case SceneHierarchy::Mode::World:
			{
				mode = ImGuizmo::WORLD;
				break;
			}
		}

		ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;

		switch (myCurrentOperation)
		{
			case SceneHierarchy::Operation::Translate:
			{
				operation = ImGuizmo::TRANSLATE;
				break;
			}
			case SceneHierarchy::Operation::Rotate:
			{
				operation = ImGuizmo::ROTATE;
				break;
			}
			case SceneHierarchy::Operation::Scale:
			{
				operation = ImGuizmo::SCALE;
				break;
			}
		}

		ImGuizmo::Manipulate(
			myCameraReference->Transform.GetFastInverse().GetData(),
			myCameraReference->RenderCamera.GetProjection().GetData(),
			operation,
			mode,
			matrix
		);

		switch (myCurrentMode)
		{
			case SceneHierarchy::Mode::Local:
			{
				transform->SetLocalMatrix(CommonUtilities::Matrix4x4f(matrix));
				break;
			}
			case SceneHierarchy::Mode::World:
			{
				transform->SetWorldMatrix(CommonUtilities::Matrix4x4f(matrix));
				break;
			}
		}
	}
}

void SceneHierarchy::SetSelectedActor(Actor* aTarget)
{
	if (mySelectedActor == aTarget)
	{
		return;
	}

	myActorsToOpen.clear();
	mySelectedActor = aTarget;

	if (mySelectedActor == nullptr)
	{
		GEngineContext.GameCamera->SetTarget(nullptr);
		GEngineContext.GameCamera->SetMode(CameraMode::FreeLook);
		return;
	}

	Actor* parent = mySelectedActor->GetParent();

	while (parent != nullptr)
	{
		myActorsToOpen.insert(parent);
		parent = parent->GetParent();
	}

	SPDLOG_INFO("Selected : {}", mySelectedActor->GetName());

	LogActorsChildren(mySelectedActor);

	// TODO: Show visualy that a actor is selected.
}

void SceneHierarchy::SetCameraReference(const CameraData* aCamera)
{
	myCameraReference = aCamera;
}

Actor* SceneHierarchy::GetSelectedActor()
{
	return mySelectedActor;
}

void SceneHierarchy::DisplayActorNode(Actor* aActor)
{
	ImGuiTreeNodeFlags flags = 
		ImGuiTreeNodeFlags_SpanAvailWidth | 
		ImGuiTreeNodeFlags_DrawLinesToNodes |
		ImGuiTreeNodeFlags_OpenOnArrow;

	if (aActor->GetChildren().empty())
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	if (mySelectedActor == aActor)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (myActorsToOpen.contains(aActor))
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	}

	bool isOpen = ImGui::TreeNodeEx(aActor, flags, "%s", aActor->GetName().c_str());

	if (ImGui::IsItemClicked())
	{
		SetSelectedActor(aActor);
	}

	// TODO: Allow actor to be dragged to root aka no parent.

	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("NODE_PAYLOAD", &aActor, sizeof(Actor*));
		ImGui::Text("%s", aActor->GetName().c_str());
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NODE_PAYLOAD"))
		{
			Actor* draggedActor = *(Actor**)payload->Data;

			if (draggedActor != aActor)
			{
				draggedActor->SetParent(aActor);
			}
		}

		ImGui::EndDragDropTarget();
	}

	if (isOpen)
	{
		for (const auto& child : aActor->GetChildren())
		{
			DisplayActorNode(child);
		}

		ImGui::TreePop();
	}
}

void SceneHierarchy::LogActorsChildren(Actor* aActor)
{
	for (const auto& child : aActor->GetChildren())
	{
		SPDLOG_INFO("Child : {}, Owned by : {}", child->GetName(), aActor->GetName());

		LogActorsChildren(child);
	}
}
#include "Tools/Tools.pch.h"
#include "SceneHierarchy.h"

#include <ImGui/imgui.h>

#include <Wrappers/Logger/LoggerWrapper.h>

#include <Core/Scene/World/Actor.hpp>
#include <Core/Scene/World/GameWorld.h>

void SceneHierarchy::Update(float aDeltaTime)
{
	aDeltaTime;
	if (mySelectedActor == nullptr)
	{

	}

}

void SceneHierarchy::Render()
{
	ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	
	for (auto& actor : GameWorld::Get().GetActors())
	{
		if (actor->GetParent() == nullptr)
		{
			DisplayActorNode(actor);
		}
	}
	
	ImGui::End();
}

void SceneHierarchy::SetSelectedActor(Actor* aTarget)
{
	mySelectedActor = aTarget;

	SPDLOG_INFO("Selected : {}", mySelectedActor->GetName());

	DebugPrintNodesChildren(mySelectedActor);
}

void SceneHierarchy::DisplayActorNode(Actor* aActor)
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
		SetSelectedActor(aActor);
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

void SceneHierarchy::DebugPrintNodesChildren(Actor* aActor)
{
	for (const auto& child : aActor->GetChildren())
	{
		SPDLOG_INFO("Child : {}, Owned by : {}", child->GetName(), aActor->GetName());

		DebugPrintNodesChildren(child);
	}
}
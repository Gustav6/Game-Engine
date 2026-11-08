#include "Tools/Tools.pch.h"
#include "SceneHierarchy.h"

#include <iostream>

#include <ImGui/imgui.h>

#include <Core/Scene/World/Actor.hpp>
#include <Core/Scene/World/GameWorld.h>

void SceneHierarchy::Update(float aDeltaTime)
{
	aDeltaTime;
}

void SceneHierarchy::Render()
{
	ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	
	for (auto& actor : GameWorld::Get().GetActors())
	{
		if (actor->GetParent() == nullptr)
		{
			DisplayActorNode(actor.get());
		}
	}
	
	ImGui::End();
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
		mySelectedActor = aActor;

		std::cout << mySelectedActor->GetName() << ", Selected" << "\n";

		DebugPrintNodesChildren(mySelectedActor);
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
		std::cout << child->GetName() << ", Owned by - " << aActor->GetName() << "\n";

		DebugPrintNodesChildren(child);
	}
}
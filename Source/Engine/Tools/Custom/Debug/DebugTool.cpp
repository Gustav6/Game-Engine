#include "Tools/Tools.pch.h"
#include "DebugTool.h"

#include <ImGui/imgui.h>

#include <Graphics/GraphicsEngine.hpp>
#include <Core/Managers/DebugSettingsManager.h>

void DebugTool::SetInfo(const DebugInfo& aInfo)
{
	myInfo = aInfo;
}

void DebugTool::Update(float aDeltaTime)
{
	aDeltaTime;
}

void DebugTool::Render()
{
	ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::CollapsingHeader("Frame Info"))
	{
		ImGui::Text("FPS: %.1f", myInfo.FPS);
		ImGui::Text("Frame Time: %.1f", myInfo.FrameTime);
		ImGui::Text("Draw Call Count: %u", myInfo.DrawCallCount);
		ImGui::Text("Pipeline Switches: %u", myInfo.PipelineSwitches);
		ImGui::Text("Visible Objects: %u", myInfo.VisibleObjects);
		ImGui::Text("Culled Objects: %u", myInfo.CulledObjects);
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

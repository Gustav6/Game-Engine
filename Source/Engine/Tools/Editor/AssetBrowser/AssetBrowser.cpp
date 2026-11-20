#include "Tools/Tools.pch.h"
#include "AssetBrowser.h"

#include <Core/EngineContext.h>

#include <imgui/imgui.h>

AssetBrowser::AssetBrowser()
{
    myCurrentFolder = GEngineContext.ContentRoot;
}

void AssetBrowser::Update(float aDeltaTime)
{
	aDeltaTime;
}

void AssetBrowser::Render()
{
	ImGui::Begin("Asset Browser");

    float padding = 16.0f;
    float thumbnailSize = 96.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1)
    {
        columnCount = 1;
    }

    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedSame;

    if (ImGui::BeginTable("AssetGrid", columnCount, flags))
    {
        for (int col = 0; col < columnCount; col++)
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, cellSize);
        }

        int index = 0;
        for (const auto& item : std::filesystem::directory_iterator(myCurrentFolder))
        {
            if (item.exists())
            {
                std::string assetName = item.path().stem().string();

                ImGui::TableNextColumn();
                ImGui::PushID(index++);

                float columnWidth = ImGui::GetContentRegionAvail().x;

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - thumbnailSize) * 0.5f);
                ImGui::Dummy(ImVec2(thumbnailSize, thumbnailSize));

                float textWidth = ImGui::CalcTextSize(assetName.c_str()).x;

                if (textWidth < columnWidth)
                {
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textWidth) * 0.5f);
                }

                ImGui::TextWrapped("%s", assetName.c_str());

                ImGui::PopID();
            }
        }

        ImGui::EndTable();
    }

	ImGui::End();
}

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

    for (const auto& item : std::filesystem::recursive_directory_iterator(myCurrentFolder))
    {
        if (item.is_regular_file() && item.path().has_extension())
        {
            std::string assetName = item.path().stem().string();

            ImGui::Text(assetName.c_str());
        }
    }

	ImGui::End();
}

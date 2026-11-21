
#include "Tools/Tools.pch.h"
#include "AssetBrowser.h"

#include <Graphics/Objects/GPU/Texture.h>

#include <Core/EngineContext.h>

#include <Core/Assets/Registry/AssetRegistry.h>
#include <Core/Assets/Concrete/TextureAsset.h>

#include <imgui/imgui.h>

#include <algorithm>
#include <cctype>
#include <vector>

constexpr int ImageSize = 64;
constexpr int GridColumns = 8;

AssetBrowser::AssetBrowser()
{
    myCurrentFolder = GEngineContext.ContentRoot;
    OpenFolderInTree(myCurrentFolder);

    auto& registry = AssetRegistry::Get();

    myAssetTypeLookup["unknown"] =
    {
        registry.GetAsset<TextureAsset>("File"),
        [](const std::filesystem::path&)
        {
            // Unknown files have no double-click action.
        }
    };

    myAssetTypeLookup["dir"] =
    {
        registry.GetAsset<TextureAsset>("Folder"),
        [this](const std::filesystem::path& path)
        {
            myCurrentFolder = path;
            OpenFolderInTree(path);
        }
    };

    myAssetTypeLookup[".png"] =
    {
        registry.GetAsset<TextureAsset>("Image"),
        [](const std::filesystem::path&)
        {
            // Open image asset.
        }
    };

    myAssetTypeLookup[".dds"] =
    {
        registry.GetAsset<TextureAsset>("Image"),
        [](const std::filesystem::path&)
        {
            // Open image asset.
        }
    };

    myAssetTypeLookup[".fbx"] =
    {
        registry.GetAsset<TextureAsset>("Mesh"),
        [](const std::filesystem::path&)
        {
            // Open model asset.
        }
    };

    myAssetTypeLookup[".json"] =
    {
        registry.GetAsset<TextureAsset>("TextFile"),
        [](const std::filesystem::path&)
        {
            // Open JSON asset.
        }
    };

    myAssetTypeLookup["back"] =
    {
        registry.GetAsset<TextureAsset>("Back"),
        [this](const std::filesystem::path& path)
        {
            myCurrentFolder = path;
            OpenFolderInTree(path);
        }
    };
}

void AssetBrowser::Update(float aDeltaTime)
{
    aDeltaTime;
}

void AssetBrowser::Render()
{
    ImGui::Begin("Asset Browser");

    const float availableWidth = ImGui::GetContentRegionAvail().x;

    // Folder tree.
    ImGui::BeginChild("FolderTree", ImVec2(availableWidth * 0.25f, 0), ImGuiChildFlags_Borders);

    ImGui::TextUnformatted("Folders");
    ImGui::Separator();

    RenderFolderTree(GEngineContext.ContentRoot);

    ImGui::EndChild();

    ImGui::SameLine();

    // Asset grid.
    ImGui::BeginChild("AssetGrid", ImVec2(0, 0), ImGuiChildFlags_Borders);

    ImGui::Text("%s", myCurrentFolder.filename().string().c_str());

    ImGui::Separator();

    RenderAssetGrid();

    ImGui::EndChild();

    ImGui::End();
}

void AssetBrowser::RenderFolderTree(const std::filesystem::path& aPath)
{
    const std::string folderName =
        aPath == GEngineContext.ContentRoot
        ? "Content"
        : aPath.filename().string();

    const bool isSelected = aPath == myCurrentFolder;

    // Check whether this folder has any entries.
    bool hasChildren = false;

    for (const auto& entry :
        std::filesystem::directory_iterator(aPath))
    {
        if (entry.is_directory() || entry.is_regular_file())
        {
            hasChildren = true;
            break;
        }
    }

    ImGuiTreeNodeFlags flags = 
        ImGuiTreeNodeFlags_OpenOnArrow | 
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_DrawLinesToNodes;

    if (isSelected)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (!hasChildren)
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    ImGui::PushID(aPath.string().c_str());

    if (myFoldersToOpen.contains(aPath))
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
    }

    // Folder tree node.
    const bool isOpen = ImGui::TreeNodeEx(
        "##FolderNode",
        flags
    );

    if (ImGui::IsItemClicked())
    {
        myCurrentFolder = aPath;
    }

    // Folder icon.
    auto folderIt = myAssetTypeLookup.find("dir");

    if (folderIt != myAssetTypeLookup.end() && folderIt->second.Icon)
    {
        const auto texture = folderIt->second.Icon->GetTexture();

        if (texture)
        {
            ImGui::SameLine();

            ImGui::Image((ImTextureRef)texture->GetShaderResourceHandle(), ImVec2(16.0f, 16.0f));
        }
    }

    ImGui::SameLine();

    ImGui::TextUnformatted(folderName.c_str());

    if (ImGui::IsItemClicked())
    {
        myCurrentFolder = aPath;
    }

    if (isOpen)
    {
        std::vector<std::filesystem::directory_entry> entries;

        for (const auto& entry : std::filesystem::directory_iterator(aPath))
        {
            if (entry.is_directory() || entry.is_regular_file())
            {
                entries.push_back(entry);
            }
        }

        // Sort folders first, then files alphabetically.
        std::ranges::sort(entries,
            [](const auto& a, const auto& b)
            {
                if (a.is_directory() != b.is_directory())
                    return a.is_directory();

                return a.path().filename().string() <
                    b.path().filename().string();
            });

        for (const auto& entry : entries)
        {
            if (entry.is_directory())
            {
                // Recursively render folders.
                RenderFolderTree(entry.path());
            }
            else
            {
                // Determine file type.
                std::string type = entry.path().extension().string();
                std::ranges::transform(type, type.begin(), tolower);

                auto it = myAssetTypeLookup.find(type);

                // Use fallback for unknown extensions.
                if (it == myAssetTypeLookup.end() || !it->second.Icon)
                {
                    it = myAssetTypeLookup.find("unknown");
                }

                if (it == myAssetTypeLookup.end() || !it->second.Icon)
                {
                    continue;
                }

                ImGui::PushID(entry.path().string().c_str());

                const auto texture = it->second.Icon->GetTexture();

                if (texture != nullptr)
                {
                    ImGui::Image((ImTextureRef)texture->GetShaderResourceHandle(), ImVec2(16.0f, 16.0f));

                    ImGui::SameLine();
                }

                // File name.
                const std::string name = entry.path().filename().string();

                ImGui::TextUnformatted(name.c_str());

                // Double-click handling.
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (it->second.OnDoubleClick)
                    {
                        it->second.OnDoubleClick(entry.path());
                    }
                }

                ImGui::PopID();
            }
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void AssetBrowser::RenderAssetGrid()
{
    std::vector<std::filesystem::directory_entry> entries;

    // Collect folder contents.
    for (const auto& item : std::filesystem::directory_iterator(myCurrentFolder))
    {
        if (item.is_directory() || item.is_regular_file())
        {
            entries.push_back(item);
        }
    }

    // Sort folders first, then alphabetically.
    std::ranges::sort(entries,
        [](const auto& a, const auto& b)
        {
            if (a.is_directory() != b.is_directory())
            {
                return a.is_directory();
            }

            return a.path().filename().string() < b.path().filename().string();
        });

    // Render asset grid.
    if (ImGui::BeginTable("AssetGridTable", GridColumns, ImGuiTableFlags_SizingStretchSame))
    {
        if (myCurrentFolder != GEngineContext.ContentRoot)
        {
            ImGui::TableNextColumn();
            RenderAsset(myCurrentFolder.parent_path(), "back");
        }

        for (const auto& item : entries)
        {
            std::string type;

            if (item.is_directory())
            {
                type = "dir";
            }
            else
            {
                type = item.path().extension().string();
                std::ranges::transform(type, type.begin(), tolower);

                if (myAssetTypeLookup.find(type) == myAssetTypeLookup.end())
                {
                    type = "unknown";
                }
            }

            ImGui::TableNextColumn();

            RenderAsset(item.path(), type);
        }

        ImGui::EndTable();
    }
}

void AssetBrowser::RenderAsset(const std::filesystem::path& path, const std::string& aType)
{
    auto it = myAssetTypeLookup.find(aType);

    if (it == myAssetTypeLookup.end() || !it->second.Icon)
    {
        it = myAssetTypeLookup.find("unknown");
    }

    // Ensure the fallback exists and has an icon.
    if (it == myAssetTypeLookup.end() || !it->second.Icon)
    {
        return;
    }

    const auto texture = it->second.Icon->GetTexture();

    if (texture == nullptr)
    {
        return;
    }

    const std::string name = path.stem().string();

    ImGui::PushID(path.string().c_str());

    // Center asset content inside its table cell.
    const float cellWidth = ImGui::GetColumnWidth();

    const float contentWidth = static_cast<float>(ImageSize);

    const float offset = (cellWidth - contentWidth) * 0.5f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

    ImGui::BeginGroup();

    // Asset icon.
    ImGui::ImageButton(
        "##AssetIcon",
        (ImTextureRef)texture->GetShaderResourceHandle(),
        ImVec2(ImageSize, ImageSize));

    // Double-click handling.
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        if (it->second.OnDoubleClick)
        {
            it->second.OnDoubleClick(path);
        }
    }

    // Asset name.
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - offset);

    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + cellWidth);

    const float textWidth = ImGui::CalcTextSize(name.c_str()).x;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellWidth - textWidth) * 0.5f);

    ImGui::Text("%s", name.c_str());

    ImGui::PopTextWrapPos();

    ImGui::EndGroup();

    ImGui::PopID();
}

void AssetBrowser::OpenFolderInTree(const std::filesystem::path& aPath)
{
    myFoldersToOpen.clear();

    std::filesystem::path folder = aPath;

    while (folder != GEngineContext.ContentRoot && folder != folder.root_path())
    {
        myFoldersToOpen.insert(folder);
        folder = folder.parent_path();
    }

    myFoldersToOpen.insert(GEngineContext.ContentRoot);
}
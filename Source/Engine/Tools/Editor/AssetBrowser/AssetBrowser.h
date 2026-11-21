#pragma once

#include <filesystem>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include <Tools/Core/Interface/ITool.h>

class TextureAsset;

class AssetBrowser : public ITool
{
public:
	AssetBrowser();

	void Update(float aDeltaTime) override;
	void Render() override;

private:
	struct AssetTypeInfo
	{
		std::shared_ptr<TextureAsset> Icon;
		std::function<void(const std::filesystem::path&)> OnDoubleClick;
	}; 
	
	void RenderAsset(const std::filesystem::path& path, const std::string& aType);
	void RenderFolderTree(const std::filesystem::path& aPath);

	void RenderAssetGrid();
	void OpenFolderInTree(const std::filesystem::path& aPath);
	
	std::filesystem::path myCurrentFolder;

	std::unordered_map<std::string, AssetTypeInfo> myAssetTypeLookup;
	std::unordered_set<std::filesystem::path> myFoldersToOpen;
};


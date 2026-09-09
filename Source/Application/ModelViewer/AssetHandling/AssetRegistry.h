#pragma once

#include <memory>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <type_traits>
#include <vector>
#include <filesystem>
#include <ranges>

#include "ensure.h"

#include <AssetHandling/Asset/Base/Asset.h>

class AssetRegistry
{
public:
	using MakeAssetFunction = std::function<std::shared_ptr<Asset>()>;

	AssetRegistry();
	~AssetRegistry();

	void Initialize(bool aLoadAllFilesRecursively);

	const std::filesystem::path& GetWorkingDirectory();

	template<typename AssetClass>
	requires std::is_base_of_v<Asset, AssetClass>
	std::shared_ptr<AssetClass> GetAsset(const std::string_view aAssetName)
	{
		std::string lowerName(aAssetName);
		std::ranges::transform(lowerName, lowerName.begin(), tolower);

		ensure(myAssets.contains(lowerName));
		AssetInformation& assetInformation = myAssets.at(lowerName);

		if (assetInformation.Asset.expired())
		{
			const std::string extension = assetInformation.Path.extension().string();

			if (myFileExtensionToAssetTypeMap.contains(extension))
			{
				const std::vector<MakeAssetFunction>& loaders = myFileExtensionToAssetTypeMap.at(extension);

				for (const auto& loader : loaders)
				{
					std::shared_ptr<Asset> asset = loader();

					if (asset->Load(assetInformation.Path))
					{
						asset->myName = assetInformation.Path.stem().string();
						assetInformation.Asset = asset;

						return std::dynamic_pointer_cast<AssetClass>(asset);
					}
				}
			}
		}

		return std::dynamic_pointer_cast<AssetClass>(assetInformation.Asset.lock());
	}

	template<typename AssetClass>
	requires std::is_base_of_v<Asset, AssetClass>
	void RegisterAssetType(const std::vector<std::string>& aExtensionList)
	{
		MakeAssetFunction function = []() { return std::make_shared<AssetClass>(); };
		RegisterAssetTypeInternal(aExtensionList, function);
	}

private:
	void RegisterAssetTypeInternal(const std::vector<std::string>& aExtensionList, const MakeAssetFunction& aMakeAssetFunction);

	void InitUnitCube();
	void InitUnitPlane();

	// TODO: Not implemented
	void InitUnitAxes();

	struct AssetInformation
	{
		std::filesystem::path Path;
		std::weak_ptr<Asset> Asset;
	};

	// Just holds assets alive aslong as the registry is not destroyed
	std::unordered_map<std::string, std::shared_ptr<Asset>> myPersistentAssets;

	std::unordered_map<std::string, AssetInformation> myAssets;
	std::unordered_map<std::string, std::vector<MakeAssetFunction>> myFileExtensionToAssetTypeMap;

	std::filesystem::path myWorkingDirectory;
};

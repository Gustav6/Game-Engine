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
#include <typeindex>
#include <set>
#include <algorithm>
#include <format>

#include "AssetPipeline/Assets/Base/Asset.h"

#include "Logger/Logger.h"

// TODO: Stop using string and move to interned string.

class AssetRegistry
{
public:
	static AssetRegistry& Get();

	AssetRegistry();
	~AssetRegistry();

	bool Initialize(std::filesystem::path aWorkingDirectory);
	void UnInitialize();

	const std::filesystem::path& GetWorkingDirectory() const;

	template<typename AssetType>
	requires std::is_base_of_v<Asset, AssetType>
	std::shared_ptr<AssetType> GetAsset(const std::string_view aAssetName)
	{
		std::string lowerName(aAssetName);
		std::ranges::transform(lowerName, lowerName.begin(), tolower);

		const std::string_view typeName = RemoveTypePrefix(typeid(AssetType).name());

		auto assetEntry = myAssets.find(lowerName);

		if (assetEntry == myAssets.end())
		{
			spdlog::error("Type [{}], Asset Name [{}], Is not contained within the content folder!", typeName, aAssetName);
			return nullptr;
		}

		AssetInformation& assetInformation = assetEntry->second;

		if (!assetInformation.Asset.expired())
		{
			return std::dynamic_pointer_cast<AssetType>(assetInformation.Asset.lock());
		}

		std::string extension = assetInformation.Path.extension().string();
		std::ranges::transform(extension, extension.begin(), tolower);

		auto extensionEntry = myFileExtensionToAssetTypeMap.find(extension);

		if (extensionEntry == myFileExtensionToAssetTypeMap.end() || !extensionEntry->second.contains(std::type_index(typeid(AssetType))))
		{
			spdlog::error("Asset Name [{}], Extension [{}], Was never registered for type {}!", aAssetName, extension, typeName);
			return nullptr;
		}

		spdlog::info("Loading Type [{}], Asset Name [{}], Extension [{}]", typeName, aAssetName, extension);

		std::shared_ptr<AssetType> asset = std::make_shared<AssetType>();

		if (!asset->Load(assetInformation.Path))
		{
			spdlog::error("Failed to load!");
			return nullptr;
		}

		assetInformation.Asset = asset;

		spdlog::info("Loaded successfully!");

		return asset;
	}

	template<typename AssetType, typename... Args>
	requires std::is_base_of_v<Asset, AssetType>
	std::shared_ptr<AssetType> ConstructAsset(const std::string_view aAssetName, Args&&... args)
	{
		std::string lowerName(aAssetName);
		std::ranges::transform(lowerName, lowerName.begin(), tolower);

		if (auto existing = myAssets.find(lowerName); existing != myAssets.end())
		{
			if (!existing->second.Asset.expired())
			{
				return std::dynamic_pointer_cast<AssetType>(existing->second.Asset.lock());
			}
		}

		const std::string_view typeName = RemoveTypePrefix(typeid(AssetType).name());

		//LOG_GROUP(LogAssetRegistry, Verbose, "Constructing Type [{}], Asset Name [{}]", typeName, aAssetName);

		std::shared_ptr<AssetType> asset = std::make_shared<AssetType>();

		if (!asset->Construct(std::forward<Args>(args)...))
		{
			spdlog::error("Failed to construct!");
			return nullptr;
		}

		asset->myName = lowerName;

		auto entry = myAssets.find(lowerName);

		if (entry != myAssets.end())
		{
			entry->second.Asset = asset;
		}
		else
		{
			myAssets.emplace(lowerName, AssetInformation{ .Path = {}, .Asset = asset });
		}

		spdlog::info("Constructed successfully!");

		return asset;
	}

	template<typename AssetClass>
	requires std::is_base_of_v<Asset, AssetClass>
	void RegisterAssetType(const std::vector<std::string>& aExtensionList)
	{
		RegisterAssetTypeInternal(aExtensionList, std::type_index(typeid(AssetClass)));
	}

private:
	struct AssetInformation
	{
		std::filesystem::path Path;
		std::weak_ptr<Asset> Asset;
	};

	void RegisterAssetTypeInternal(const std::vector<std::string>& aExtensionList, std::type_index aTypeIndex);

	std::string_view RemoveTypePrefix(std::string_view aTypeName);

	// TODO: Not implemented
	void InitUnitAxes();
	void InitUnitCube();
	void InitUnitSphere();
	void InitUnitPlane();

	// Just holds assets alive as long as the registry is not destroyed
	std::vector<std::shared_ptr<Asset>> myPersistentAssets;

	std::unordered_map<std::string, AssetInformation> myAssets;
	std::unordered_map<std::string, std::set<std::type_index>> myFileExtensionToAssetTypeMap;

	std::filesystem::path myWorkingDirectory;
};

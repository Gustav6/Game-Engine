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

#include "Logger/Logger.h"
#include "AssetPipeline/Assets/Base/Asset.h"

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

		if (!myAssets.contains(lowerName))
		{
			std::string errorMessage = std::format("Type [{}], Asset Name [{}], Is not contained within the content folder!", RemoveTypePrefix(typeid(AssetType).name()), aAssetName);
			InternalLogMessage(LogVerbosity::Type::Error, errorMessage);
			return nullptr;
		}

		AssetInformation& assetInformation = myAssets.at(lowerName);

		if (!assetInformation.Asset.expired())
		{
			return std::dynamic_pointer_cast<AssetType>(assetInformation.Asset.lock());
		}

		std::string extension = assetInformation.Path.extension().string();
		auto extensionEntry = myFileExtensionToAssetTypeMap.find(extension);

		if (extensionEntry == myFileExtensionToAssetTypeMap.end() || !extensionEntry->second.contains(std::type_index(typeid(AssetType))))
		{
			std::string errorMessage = std::format("Asset Name [{}], Extension [{}], Was never registered for type {}!", aAssetName, extension, RemoveTypePrefix(typeid(AssetType).name()));
			InternalLogMessage(LogVerbosity::Type::Error, errorMessage);
			return nullptr;
		}

		std::shared_ptr<AssetType> asset = std::make_shared<AssetType>();

		if (!asset->Load(assetInformation.Path))
		{
			return nullptr;
		}

		assetInformation.Asset = asset;

		std::string loadMessage = std::format("Type [{}], Asset Name [{}], Extension [{}], Loaded successfully!", RemoveTypePrefix(typeid(AssetType).name()), aAssetName, extension);
		InternalLogMessage(LogVerbosity::Type::Verbose, loadMessage);

		return asset;
	}

	template<typename AssetClass>
	requires std::is_base_of_v<Asset, AssetClass>
	void RegisterAssetType(const std::vector<std::string>& aExtensionList)
	{
		RegisterAssetTypeInternal(aExtensionList, std::type_index(typeid(AssetClass)));
	}

private:
	std::string_view RemoveTypePrefix(std::string_view aTypeName);

	void RegisterAssetTypeInternal(const std::vector<std::string>& aExtensionList, std::type_index aTypeIndex);

	void InternalLogMessage(LogVerbosity::Type aVerbosity, std::string_view aMessage) const;

	// TODO: Not implemented
	void InitUnitAxes();
	void InitUnitCube();
	// TODO: Not implemented
	void InitUnitSphere();
	void InitUnitPlane();

	struct AssetInformation
	{
		std::filesystem::path Path;
		std::weak_ptr<Asset> Asset;
	};

	// Just holds assets alive as long as the registry is not destroyed
	std::vector<std::shared_ptr<Asset>> myPersistentAssets;

	std::unordered_map<std::string, AssetInformation> myAssets;
	std::unordered_map<std::string, std::set<std::type_index>> myFileExtensionToAssetTypeMap;

	std::filesystem::path myWorkingDirectory;
};

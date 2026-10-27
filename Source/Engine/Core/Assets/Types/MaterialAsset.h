#pragma once

#include <filesystem>
#include <vector>

#include "Core/Assets/Base/Asset.h"

#include "Wrappers/Json/JsonWrapper.h"

class MaterialInterface;
class TextureAsset;

enum class MaterialLayout : uint8_t;

enum class MaterialType
{
	Unreal,
	Sprite
};

class MaterialAsset : public Asset
{
	friend class AssetRegistry;

public:
	const std::shared_ptr<MaterialInterface>& GetMaterial() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	bool Construct(const Json::Data& aMaterialData, MaterialType aMaterialType);

	bool LoadUnrealInstanceData(const Json::Data& aMaterialData);
	bool LoadUnrealMasterData(const Json::Data& aMaterialData);

	void PushParameters(const std::vector<Json::Data>& aMaterialParameters, MaterialInterface* aMaterialInstance);

	std::shared_ptr<MaterialInterface> myMaterial;
	std::vector<std::shared_ptr<TextureAsset>> myPersistentTextureList;
};
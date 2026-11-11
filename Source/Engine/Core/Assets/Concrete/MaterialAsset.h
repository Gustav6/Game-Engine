#pragma once

#include <filesystem>
#include <vector>

#include "Core/Assets/Interface/IAsset.h"

#include "Wrappers/Json/JsonWrapper.h"

class MaterialInterface;
class TextureAsset;

enum class MaterialLayout : uint8_t;

class MaterialAsset : public Asset
{
	friend class AssetRegistry;

public:
	const std::shared_ptr<MaterialInterface>& GetMaterial() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	bool Construct(const Json::Data& aMaterialData);

	bool LoadUnrealInstanceData(const Json::Data& aMaterialData);
	bool LoadUnrealMasterData(const Json::Data& aMaterialData);

	void PushParameters(const std::vector<Json::Data>& aMaterialParameters, MaterialInterface* aMaterialInstance);

	MaterialLayout GetLayout(const std::vector<Json::Data>& aMaterialParameters);
	uint8_t GetFeatures(const std::vector<Json::Data>& aMaterialParameters);

	std::shared_ptr<MaterialInterface> myMaterial;
	std::vector<std::shared_ptr<TextureAsset>> myPersistentTextureList;
};
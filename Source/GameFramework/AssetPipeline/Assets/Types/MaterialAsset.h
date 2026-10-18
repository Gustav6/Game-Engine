#pragma once

#include <filesystem>

#include "AssetPipeline/Assets/Base/Asset.h"
#include "Json/JsonWrapper.h"

class MaterialInterface;

class MaterialAsset : public Asset
{
	friend class AssetRegistry;

public:
	const std::shared_ptr<MaterialInterface>& GetMaterial() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	bool Construct(const Json::Data& aMaterialData);

	bool LoadInstanceData(const Json::Data& aMaterialData);
	bool LoadMasterData(const Json::Data& aMaterialData);
	void PushParameters(const std::vector<Json::Data>& aMaterialParameters, MaterialInterface* aMaterialInstance);

	std::shared_ptr<MaterialInterface> myMaterial;
};
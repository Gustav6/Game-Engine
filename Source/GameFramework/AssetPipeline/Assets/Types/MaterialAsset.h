#pragma once

#include <filesystem>

#include "AssetPipeline/Assets/Base/Asset.h"

class JsonData;
class MaterialInterface;

class MaterialAsset : public Asset
{
	friend class AssetRegistry;

public:
	const std::shared_ptr<MaterialInterface>& GetMaterial() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	bool Construct(const JsonData& aMaterialData);

	void PushParameters(const std::vector<JsonData>& aMaterialParameters, MaterialInterface* aMaterialInstance);

	std::shared_ptr<MaterialInterface> myMaterial;
};
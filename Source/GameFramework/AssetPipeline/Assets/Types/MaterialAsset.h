#pragma once

#include <filesystem>

#include "AssetPipeline/Assets/Base/Asset.h"

class JsonData;
class MaterialInterface;
class MaterialInstance;

class MaterialAsset : public Asset
{
	friend class AssetRegistry;

public:
	const std::shared_ptr<MaterialInterface>& GetMaterial() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	void LoadParameters(const JsonData& aMaterialParameters, MaterialInstance* aMaterialInstance);

	std::shared_ptr<MaterialInterface> myMaterial;
};


#pragma once

#include <filesystem>

#include "../Base/Asset.h"

class TextureAsset;
class JsonData;
class MaterialInterface;
class MaterialInstance;

class MaterialAsset : public Asset
{
	friend class AssetRegistry;

public:
	~MaterialAsset() override;

	const std::shared_ptr<MaterialInterface> GetMaterial() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	void LoadParameters(const JsonData& aMaterialParameters, MaterialInstance* aMaterialInstance);

	std::shared_ptr<MaterialInterface> myMaterial;
};


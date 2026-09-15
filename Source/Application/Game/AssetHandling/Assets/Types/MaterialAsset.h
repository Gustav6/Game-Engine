#pragma once

#include <filesystem>

#include "TextureAsset.h"

#include <AssetHandling/Assets/Base/Asset.h>

#include "GraphicsEngine/Materials/Material.h"

class JsonData;

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

	std::vector<std::shared_ptr<TextureAsset>> myTextureAssets;
};


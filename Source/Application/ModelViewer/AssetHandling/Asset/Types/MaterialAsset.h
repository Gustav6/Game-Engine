#pragma once

#include <filesystem>

#include "AssetHandling/Asset/Base/Asset.h"

#include "GraphicsEngine/Materials/Material.h"

class MaterialAsset : public Asset
{
	friend class AssetRegistry;

public:
	~MaterialAsset() override;

	const Material& GetMaterial() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	Material myMaterial;
};


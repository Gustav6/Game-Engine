#pragma once

#include <filesystem>

#include <AssetHandling/Assets/Base/Asset.h>

#include "GraphicsEngine/Objects/Mesh.h"
#include "GraphicsEngine/Objects/Skeleton.h"

class MeshAsset : public Asset
{
	friend class AssetRegistry;

public:
	~MeshAsset() override;

	const Mesh& GetMesh() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	Mesh myMesh;
};


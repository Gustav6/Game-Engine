#pragma once

#include <filesystem>
#include <memory>

#include "Core/Assets/Base/Asset.h"

class Mesh;

class MeshAsset : public Asset
{
	friend class AssetRegistry;

public:
	MeshAsset();
	~MeshAsset() override;

	const Mesh& GetMesh() const;

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	std::unique_ptr<Mesh> myMesh;
};


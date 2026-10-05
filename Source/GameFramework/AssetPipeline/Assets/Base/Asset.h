#pragma once

#include <filesystem>

class Asset
{
	friend class AssetRegistry;

public:
	virtual ~Asset();

protected:
	virtual bool Load(const std::filesystem::path& aPath) = 0;
};


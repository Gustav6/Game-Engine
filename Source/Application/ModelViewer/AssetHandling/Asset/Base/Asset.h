#pragma once

#include <string_view>
#include <filesystem>
#include <string>

class Asset
{
	friend class AssetRegistry;

public:
	virtual ~Asset();

	std::string_view GetName();

protected:
	virtual bool Load(const std::filesystem::path& aPath) = 0;

private:
	std::string myName;
};


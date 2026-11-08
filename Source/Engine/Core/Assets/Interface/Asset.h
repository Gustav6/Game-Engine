#pragma once

#include <filesystem>
#include <string>

class Asset
{
	friend class AssetRegistry;

public:
	virtual ~Asset();

	const std::string& GetName() const;

protected:
	virtual bool Load(const std::filesystem::path& aPath) = 0;

private:
	std::string myName;
};


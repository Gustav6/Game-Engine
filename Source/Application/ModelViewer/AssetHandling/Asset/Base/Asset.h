#pragma once

#include <string_view>
#include <filesystem>
#include <string>

#include "Logger/Logger.h"

DECLARE_LOG_CATEGORY_WITH_NAME(FBXLog, FBX, Verbose);
DECLARE_LOG_CATEGORY_WITH_NAME(TextureLog, TEXTURE, Verbose);
DECLARE_LOG_CATEGORY_WITH_NAME(MaterialLog, MATERIAL, Verbose);

//DEFINE_LOG_CATEGORY(FBXLog);
//DEFINE_LOG_CATEGORY(TextureLog);
//DEFINE_LOG_CATEGORY(MaterialLog);

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


#include "Core.pch.h"
#include "Asset.h"

Asset::~Asset() = default;

const std::string& Asset::GetName() const
{
	return myName;
}

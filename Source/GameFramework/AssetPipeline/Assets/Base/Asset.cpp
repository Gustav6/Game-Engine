#include "GameFramework.pch.h"
#include "Asset.h"

Asset::~Asset() = default;

std::string_view Asset::GetName()
{
	return myName;
}

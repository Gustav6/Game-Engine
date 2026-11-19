#pragma once

#include <filesystem>

#include <Tools/Core/Interface/ITool.h>

class AssetBrowser : public ITool
{
public:
	AssetBrowser();

	void Update(float aDeltaTime) override;
	void Render() override;

private:
	std::filesystem::path myCurrentFolder;
};


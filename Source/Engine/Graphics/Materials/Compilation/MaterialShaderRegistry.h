#pragma once

#include <unordered_map>
#include <filesystem>

#include "Graphics/Materials/Material.h"

class MaterialShaderRegistry
{
public:
    void Initialize(const std::filesystem::path& aShaderRoot);

    const std::filesystem::path& GetDomainShader(MaterialDomain aDomain) const;
    const std::filesystem::path& GetShadingModelShader(ShadingModel aModel) const;

private:
    std::unordered_map<MaterialDomain, std::filesystem::path> myMaterialDomainShaders;
    std::unordered_map<ShadingModel, std::filesystem::path> myMaterialShadingModelShaders;
};
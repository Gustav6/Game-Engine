#include "GraphicsEngine.pch.h"
#include "MaterialShaderRegistry.h"

void MaterialShaderRegistry::Initialize(const std::filesystem::path& aShaderRoot)
{
	myMaterialDomainShaders.emplace(MaterialDomain::Surface, aShaderRoot / "Geometry" / "Surface_VS.hlsl");

	myMaterialShadingModelShaders.emplace(ShadingModel::Unlit, aShaderRoot / "Forward" / "Unlit_PS.hlsl");
	myMaterialShadingModelShaders.emplace(ShadingModel::Lit, aShaderRoot / "Forward" / "PBR_PS.hlsl");
}

const std::filesystem::path& MaterialShaderRegistry::GetDomainShader(MaterialDomain aDomain) const
{
	return myMaterialDomainShaders.at(aDomain);
}

const std::filesystem::path& MaterialShaderRegistry::GetShadingModelShader(ShadingModel aModel) const
{
	return myMaterialShadingModelShaders.at(aModel);
}
#include "GraphicsEngine.pch.h"
#include "MaterialFactory.h"

#include "../MaterialShaderIncludeHandler.h"

#include "MaterialShaderRegistry.h"

#include "Materials/Material.h"

#include "GraphicsEngine/Objects/GPU/Shader.h"
#include "GraphicsEngine/Objects/GPU/Vertex.h"

#include "GraphicsEngine/RHI/RenderHardwareInterface.h"
#include "GraphicsEngine/RHI/RHIShaderReflectionInfo.h"

bool MaterialFactory::Create(
    const MaterialDescription& aDescription, 
    RenderHardwareInterface& aRHI, 
    const MaterialShaderRegistry& aShaderRegistry, 
    const std::filesystem::path& aShaderRoot, 
    Material& outMaterial)
{
	Shader materialVS;
	Shader materialPS;

	if (aDescription.BlendMode == MaterialBlendMode::None)
	{
		SPDLOG_ERROR("Material {} has invalid Blend Mode setting!", aDescription.Name);
		return false;
	}

	if (aDescription.Domain == MaterialDomain::None)
	{
		SPDLOG_ERROR("Material {} has invalid Material Domain setting!", aDescription.Name);
		return false;
	}

	if (aDescription.ShadingModel == ShadingModel::None)
	{
		SPDLOG_ERROR("Material {} has invalid Shading Model setting!", aDescription.Name);
		return false;
	}

	if (aDescription.Name.empty())
	{
		SPDLOG_ERROR("Material {} has no given name!", aDescription.Name);
		return false;
	}

	{
		std::filesystem::path shaderProgramPath;

		shaderProgramPath = aShaderRegistry.GetDomainShader(aDescription.Domain);

		const std::filesystem::path shaderRoot = aShaderRoot / "Include";
		std::filesystem::path materialProgramPath;

		if (aDescription.MaterialShaderCode.empty())
		{
			materialProgramPath = aShaderRoot / "Include" / "Core" / "Material.hlsli";
		}
		else
		{
			materialProgramPath = aDescription.MaterialShaderCode;
		}

		MaterialShaderIncludeHandler handler(shaderRoot, shaderProgramPath, materialProgramPath);

		if (!aRHI.CompileShader(ShaderType::VertexShader, shaderProgramPath, &handler, true, materialVS))
		{
			return false;
		}
	}

	{
		std::filesystem::path shaderProgramPath;

		if (aDescription.ShadingModel == ShadingModel::Unlit)
		{
			shaderProgramPath = aShaderRegistry.GetShadingModelShader(aDescription.ShadingModel);
		}
		else
		{
			switch (aDescription.BlendMode)
			{
				case MaterialBlendMode::Opaque:
				{
					shaderProgramPath = aShaderRoot / "Deferred" / "GBuffer_PS.hlsl";
					break;
				}
				case MaterialBlendMode::Alpha:
				{
					shaderProgramPath = aShaderRegistry.GetShadingModelShader(aDescription.ShadingModel);
					break;
				}
				default:
				{
					// TODO: Log error
					break;
				}
			}
		}

		const std::filesystem::path shaderRoot = aShaderRoot / "Include";
		std::filesystem::path materialProgramPath;

		if (aDescription.MaterialShaderCode.empty())
		{
			materialProgramPath = aShaderRoot / "Include" / "Core" / "Material.hlsli";
		}
		else
		{
			materialProgramPath = aDescription.MaterialShaderCode;
		}

		MaterialShaderIncludeHandler handler(shaderRoot, shaderProgramPath, materialProgramPath);

		if (!aRHI.CompileShader(ShaderType::PixelShader, shaderProgramPath, &handler, true, materialPS))
		{
			return false;
		}
	}

	RHIShaderReflectionInfo vsInfo, psInfo;
	RHIShaderReflector::Reflect(materialVS.GetDataPtr(), materialVS.GetDataSize(), vsInfo);
	RHIShaderReflector::Reflect(materialPS.GetDataPtr(), materialPS.GetDataSize(), psInfo);

	const RHIShaderReflectionInfo* materialBufferSource = nullptr;
	static std::string materialBufferName = "MaterialBuffer";

	if (vsInfo.ConstantBufferNameToIndex.contains(materialBufferName))
	{
		materialBufferSource = &vsInfo;
	}
	else if (psInfo.ConstantBufferNameToIndex.contains(materialBufferName))
	{
		materialBufferSource = &psInfo;
	}

	memset(outMaterial.myData, 0, Material::MATERIAL_BUFFER_SIZE);

	if (materialBufferSource != nullptr)
	{
		const RHIShaderReflectionInfo::ConstantBufferInfo& info = materialBufferSource->ConstantBuffers[materialBufferSource->ConstantBufferNameToIndex.at(materialBufferName)];

		for (size_t i = 0; i < info.Members.size(); i++)
		{
			const auto& member = info.Members[i];

			MaterialParameterInfo param;
			param.Name = member.Name;
			param.Type = MaterialHelpers::HLSLTypeToMaterialParameterType(member.Type);
			param.Size = member.Size;
			param.Offset = member.Offset;
			param.Index = i;

			memcpy_s(outMaterial.myData + param.Offset, param.Size, member.Default, member.Size);

			outMaterial.myParameterNameToIndex.emplace(param.Name, static_cast<unsigned int>(outMaterial.myParameters.size()));
			outMaterial.myParameters.emplace_back(std::move(param));
		}
	}

	PipelineStateDescription matPSODesc;
	matPSODesc.Name = std::format("{}_MAT_PSO", aDescription.Name);
	matPSODesc.VertexShader.ByteCode = materialVS.GetDataPtr();
	matPSODesc.VertexShader.ByteCodeSize = materialVS.GetDataSize();
	matPSODesc.PixelShader.ByteCode = materialPS.GetDataPtr();
	matPSODesc.PixelShader.ByteCodeSize = materialPS.GetDataSize();
	matPSODesc.InputLayoutElements = Vertex::Description;
	matPSODesc.Topology = Topology::TriangleList;

	switch (aDescription.BlendMode)
	{
		case MaterialBlendMode::Opaque:
		{
			matPSODesc.BlendMode = BlendMode::Opaque;
			break;
		}
		case MaterialBlendMode::Alpha:
		{
			matPSODesc.BlendMode = BlendMode::Alpha;
			break;
		}
		default:
		{
			// TODO: Log error
			break;
		}
	}

	PipelineStateObject matPSO;

	if (!aRHI.CreatePipelineStateObject(matPSODesc, matPSO))
	{
		return false;
	}

	CreateMaterialTextureSlots(vsInfo, outMaterial);
	CreateMaterialTextureSlots(psInfo, outMaterial);

	outMaterial.myPSO = matPSO;
	outMaterial.myName = aDescription.Name;
	outMaterial.myDescription = aDescription;

	return true;
}

void MaterialFactory::CreateMaterialTextureSlots(const RHIShaderReflectionInfo& aShaderInfo, Material& inoutMaterial)
{
	for (const auto& shaderTextureSlot : aShaderInfo.Bindings)
	{
		if (shaderTextureSlot.Type != 2 || shaderTextureSlot.BindPoint >= Material::MAX_MATERIAL_TEXTURE_COUNT)
		{
			continue;
		}

		std::string lowerName = shaderTextureSlot.Name;
		std::ranges::transform(lowerName, lowerName.begin(), tolower);

		if (inoutMaterial.myTexturesSlotNameToIndex.contains(lowerName) && inoutMaterial.myTexturesSlotNameToIndex.at(lowerName) != shaderTextureSlot.BindPoint)
		{
			SPDLOG_WARN("Found texture {} in multiple places when setting up material. Only the first instance will be used!", shaderTextureSlot.Name);
			continue;
		}

		inoutMaterial.myTexturesSlotNameToIndex.emplace(lowerName, shaderTextureSlot.BindPoint);
	}
}

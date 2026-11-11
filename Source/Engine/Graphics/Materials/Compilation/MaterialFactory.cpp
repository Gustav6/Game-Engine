#include "Graphics/GraphicsEngine.pch.h"
#include "MaterialFactory.h"

#include "Graphics/Materials/MaterialShaderIncludeHandler.h"

#include "MaterialShaderRegistry.h"

#include "Graphics/Materials/Material.h"

#include "Graphics/Objects/GPU/Shader.h"
#include "Graphics/Objects/GPU/Vertex.h"

#include "Graphics/RHI/RenderHardwareInterface.h"
#include "Graphics/RHI/RHIShaderReflectionInfo.h"

bool MaterialFactory::Create(
    const MaterialDescription& aDescription, 
    RenderHardwareInterface& aRHI, 
    const MaterialShaderRegistry& aShaderRegistry, 
    const std::filesystem::path& aShaderRoot, 
    Material& outMaterial)
{
	if (aDescription.Name.empty())
	{
		SPDLOG_ERROR("Material has no given name!");
		return false;
	}

	switch (aDescription.Type)
	{
		case MaterialType::Surface:
			return CreateSurfaceMaterial(aDescription, aRHI, aShaderRegistry, aShaderRoot, outMaterial);
		case MaterialType::Sprite:
			return CreateSpriteMaterial(aDescription, aRHI, aShaderRegistry, aShaderRoot, outMaterial);
		case MaterialType::Font:
			return CreateFontMaterial(aDescription, aRHI, aShaderRegistry, aShaderRoot, outMaterial);
		case MaterialType::Particle:
			return CreateParticleMaterial(aDescription, aRHI, aShaderRegistry, aShaderRoot, outMaterial);
		default:
		{
			SPDLOG_ERROR("Material {} has an invalid Material Type!", aDescription.Name);
			return false;
		}
	}
}

bool MaterialFactory::CreateSurfaceMaterial(
	const MaterialDescription& aDescription, 
	RenderHardwareInterface& aRHI, 
	const MaterialShaderRegistry& aShaderRegistry,
	const std::filesystem::path& aShaderRoot,
	Material& outMaterial)
{
	if (aDescription.Domain == MaterialDomain::None)
	{
		SPDLOG_ERROR("Material {} has invalid Domain!", aDescription.Name);
		return false;
	}

	if (aDescription.ShadingModel == ShadingModel::None)
	{
		SPDLOG_ERROR("Material {} has invalid Shading Model!", aDescription.Name);
		return false;
	}

    if (aDescription.BlendMode == MaterialBlendMode::None)
    {
        SPDLOG_ERROR("Material {} has invalid Blend Mode!", aDescription.Name);
        return false;
    }

	if (aDescription.Layout == MaterialLayout::None)
	{
		SPDLOG_ERROR("Material {} has invalid Layout!", aDescription.Name);
		return false;
	}

	std::vector<std::pair<std::string, std::string>> macros;

	switch (aDescription.Layout)
	{
		case MaterialLayout::PBR:
		{
			macros.emplace_back("MATERIAL_LAYOUT_PBR", "1");
			break;
		}
		case MaterialLayout::Splat:
		{
			macros.emplace_back("MATERIAL_LAYOUT_SPLAT", "1");
			break;
		}
	}

	if (aDescription.Features & MaterialFeatureFlag_Emission)
	{
		macros.emplace_back("MATERIAL_HAS_EMISSION", "1");
	}

	if (aDescription.Features & MaterialFeatureFlag_HeightMap)
	{
		macros.emplace_back("MATERIAL_HAS_HEIGHTMAP", "1");
	}

	ShaderProgramDescription program;

	if (!aShaderRegistry.Resolve(aDescription, program))
	{
		return false;
	}

	return CreateFromProgram(aDescription, program, macros, aRHI, aShaderRoot, outMaterial);
}

bool MaterialFactory::CreateSpriteMaterial(
	const MaterialDescription& aDescription, 
	RenderHardwareInterface& aRHI, 
	const MaterialShaderRegistry& aShaderRegistry,
	const std::filesystem::path& aShaderRoot,
	Material& outMaterial)
{
	std::vector<std::pair<std::string, std::string>> macros;

	ShaderProgramDescription program;

	if (!aShaderRegistry.Resolve(aDescription, program))
	{
		return false;
	}

	return CreateFromProgram(aDescription, program, macros, aRHI, aShaderRoot, outMaterial);
}

bool MaterialFactory::CreateFontMaterial(
	const MaterialDescription& aDescription, 
	RenderHardwareInterface& aRHI, 
	const MaterialShaderRegistry& aShaderRegistry,
	const std::filesystem::path& aShaderRoot,
	Material& outMaterial)
{
	std::vector<std::pair<std::string, std::string>> macros;

	ShaderProgramDescription program;

	if (!aShaderRegistry.Resolve(aDescription, program))
	{
		return false;
	}

	return CreateFromProgram(aDescription, program, macros, aRHI, aShaderRoot, outMaterial);
}

bool MaterialFactory::CreateParticleMaterial(
	const MaterialDescription& aDescription, 
	RenderHardwareInterface& aRHI, 
	const MaterialShaderRegistry& aShaderRegistry,
	const std::filesystem::path& aShaderRoot,
	Material& outMaterial)
{
	std::vector<std::pair<std::string, std::string>> macros;

	ShaderProgramDescription program;

	if (!aShaderRegistry.Resolve(aDescription, program))
	{
		return false;
	}

	return CreateFromProgram(aDescription, program, macros, aRHI, aShaderRoot, outMaterial);
}

bool MaterialFactory::CreateFromProgram(
    const MaterialDescription& aDescription,
    const ShaderProgramDescription& aProgram,
    const std::vector<std::pair<std::string, std::string>>& aMacros,
    RenderHardwareInterface& aRHI,
    const std::filesystem::path& aShaderRoot,
    Material& outMaterial)
{
    if (aProgram.VertexShader.empty() || aProgram.PixelShader.empty())
    {
        SPDLOG_ERROR("Material {} has an incomplete shader program!", aDescription.Name);
        return false;
    }

    if (aDescription.BlendMode == MaterialBlendMode::None)
    {
        SPDLOG_ERROR("Material {} has invalid Blend Mode!", aDescription.Name);
        return false;
    }

    std::filesystem::path materialProgramPath;

    if (aDescription.MaterialShaderCode.empty())
    {
        materialProgramPath =
            aShaderRoot / "Include" / "Core" / "Material.hlsli";
    }
    else
    {
        materialProgramPath = aDescription.MaterialShaderCode;
    }

    const std::filesystem::path shaderRoot = aShaderRoot / "Include";

    Shader materialVS;

    {
        MaterialShaderIncludeHandler handler(shaderRoot, aProgram.VertexShader, materialProgramPath);

        if (!aRHI.CompileShader(ShaderType::VertexShader, aProgram.VertexShader, &handler, true, materialVS, aMacros))
        {
            SPDLOG_ERROR("Failed to compile vertex shader for material {}!", aDescription.Name);
            return false;
        }
    }

    Shader materialPS;

    {
        MaterialShaderIncludeHandler handler(shaderRoot, aProgram.PixelShader, materialProgramPath);

        if (!aRHI.CompileShader(ShaderType::PixelShader, aProgram.PixelShader, &handler, true, materialPS, aMacros))
        {
            SPDLOG_ERROR("Failed to compile pixel shader for material {}!", aDescription.Name);
            return false;
        }
    }

    RHIShaderReflectionInfo vsInfo;
    RHIShaderReflectionInfo psInfo;

    RHIShaderReflector::Reflect(materialVS.GetDataPtr(), materialVS.GetDataSize(), vsInfo);

    RHIShaderReflector::Reflect(materialPS.GetDataPtr(), materialPS.GetDataSize(), psInfo);

    InitializeMaterialParameters(vsInfo, psInfo, outMaterial);

    PipelineStateDescription matPSODesc;
    matPSODesc.Name = std::format("{}_MAT_PSO", aDescription.Name);
    matPSODesc.VertexShader.ByteCode = materialVS.GetDataPtr();
    matPSODesc.VertexShader.ByteCodeSize = materialVS.GetDataSize();
    matPSODesc.PixelShader.ByteCode = materialPS.GetDataPtr();
    matPSODesc.PixelShader.ByteCodeSize = materialPS.GetDataSize();
    matPSODesc.Topology = aProgram.Topology;

    if (!GetVertexLayoutDescription(aProgram.VertexLayout, matPSODesc.InputLayoutElements))
    {
        SPDLOG_ERROR("Material {} has an invalid Vertex Layout!", aDescription.Name);

        return false;
    }

    switch (aDescription.BlendMode)
    {
        case MaterialBlendMode::Opaque:
            matPSODesc.BlendMode = BlendMode::Opaque;
            break;

        case MaterialBlendMode::Alpha:
            matPSODesc.BlendMode = BlendMode::Alpha;
            break;

        default:
            SPDLOG_ERROR("Material {} has an unsupported Blend Mode!", aDescription.Name);
            return false;
    }

    PipelineStateObject matPSO;

    if (!aRHI.CreatePipelineStateObject(matPSODesc, matPSO))
    {
        SPDLOG_ERROR("Failed to create PSO for material {}!", aDescription.Name);
        return false;
    }

    CreateMaterialTextureSlots(vsInfo, outMaterial);
    CreateMaterialTextureSlots(psInfo, outMaterial);

    outMaterial.myPSO = matPSO;
    outMaterial.myName = aDescription.Name;
    outMaterial.myDescription = aDescription;

    return true;
}

bool MaterialFactory::GetVertexLayoutDescription(
	VertexLayoutType aType, 
	std::vector<VertexElementDescription>& outElements)
{
	switch (aType)
	{
		case VertexLayoutType::Mesh:
			outElements = Vertex::Description;
			return true;

		case VertexLayoutType::Line:
			outElements = LineVertex::Description;
			return true;

		case VertexLayoutType::Particle:
			outElements = ParticleVertex::Description;
			return true;

		case VertexLayoutType::Sprite:
			outElements = SpriteVertex::Description;
			return true;

		case VertexLayoutType::Font:
			outElements = FontVertex::Description;
			return true;

		default:
			return false;
	}
}

void MaterialFactory::InitializeMaterialParameters(
    const RHIShaderReflectionInfo& aVSInfo,
    const RHIShaderReflectionInfo& aPSInfo,
    Material& outMaterial)
{
    static constexpr std::string_view materialBufferName = "MaterialBuffer";

    const RHIShaderReflectionInfo* materialBufferSource = nullptr;

    if (aVSInfo.ConstantBufferNameToIndex.contains(std::string(materialBufferName)))
    {
        materialBufferSource = &aVSInfo;
    }
    else if (aPSInfo.ConstantBufferNameToIndex.contains(std::string(materialBufferName)))
    {
        materialBufferSource = &aPSInfo;
    }

    memset(outMaterial.myData, 0, Material::MATERIAL_BUFFER_SIZE);

    outMaterial.myParameters.clear();
    outMaterial.myParameterNameToIndex.clear();

    if (materialBufferSource == nullptr)
    {
        return;
    }

	const RHIShaderReflectionInfo::ConstantBufferInfo& info = materialBufferSource->ConstantBuffers[materialBufferSource->ConstantBufferNameToIndex.at(std::string(materialBufferName))];

    for (size_t i = 0; i < info.Members.size(); i++)
    {
        const auto& member = info.Members[i];

        MaterialParameterInfo param;
        param.Name = member.Name;
        param.Type = MaterialHelpers::HLSLTypeToMaterialParameterType(member.Type);
        param.Size = member.Size;
        param.Offset = member.Offset;
        param.Index = i;

        if (param.Offset + param.Size > Material::MATERIAL_BUFFER_SIZE)
        {
            SPDLOG_ERROR("Material parameter {} exceeds material buffer size!", param.Name);
            continue;
        }

        memcpy_s(outMaterial.myData + param.Offset, Material::MATERIAL_BUFFER_SIZE - param.Offset, member.Default, member.Size);

        outMaterial.myParameterNameToIndex.emplace(param.Name, static_cast<unsigned int>(outMaterial.myParameters.size()));
        outMaterial.myParameters.emplace_back(std::move(param));
    }
}

void MaterialFactory::CreateMaterialTextureSlots(
	const RHIShaderReflectionInfo& aShaderInfo, 
	Material& inoutMaterial)
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

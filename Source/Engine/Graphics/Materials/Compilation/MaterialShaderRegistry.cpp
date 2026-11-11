#include "Graphics/GraphicsEngine.pch.h"
#include "MaterialShaderRegistry.h"

void MaterialShaderRegistry::Initialize(
    const std::filesystem::path& aShaderRoot)
{
    myPrograms.clear();

    // Surface: Unlit
    myPrograms.emplace(
        "Surface.Unlit",
        ShaderProgramDescription
        {
            .VertexShader = aShaderRoot / "Geometry/Surface_VS.hlsl",
            .PixelShader = aShaderRoot / "Forward/Unlit_PS.hlsl",
            .VertexLayout = VertexLayoutType::Mesh
        });

    // Surface: PBR
    myPrograms.emplace(
        "Surface.PBR",
        ShaderProgramDescription
        {
            .VertexShader = aShaderRoot / "Geometry/Surface_VS.hlsl",
            .PixelShader = aShaderRoot / "Deferred/GBuffer_PS.hlsl",
            .VertexLayout = VertexLayoutType::Mesh
        });

    // Surface: Transparent PBR
    myPrograms.emplace(
        "Surface.PBR.Transparent",
        ShaderProgramDescription
        {
            .VertexShader = aShaderRoot / "Geometry/Surface_VS.hlsl",
            .PixelShader = aShaderRoot / "Forward/PBR_PS.hlsl",
            .VertexLayout = VertexLayoutType::Mesh
        });

    // Sprite
    myPrograms.emplace(
        "Sprite.Default",
        ShaderProgramDescription
        {
            .VertexShader = aShaderRoot / "UI/Sprite_VS.hlsl",
            .PixelShader = aShaderRoot / "UI/Sprite_PS.hlsl",
            .VertexLayout = VertexLayoutType::Sprite
        });

    // Font
    myPrograms.emplace(
        "Font.Default",
        ShaderProgramDescription
        {
            .VertexShader = aShaderRoot / "UI/Font_VS.hlsl",
            .PixelShader = aShaderRoot / "UI/Font_PS.hlsl",
            .VertexLayout = VertexLayoutType::Font
        });

    // Particle
    myPrograms.emplace(
        "Particle.Default",
        ShaderProgramDescription
        {
            .VertexShader = aShaderRoot / "Particles/Particle_VS.hlsl",
            .PixelShader = aShaderRoot / "Particles/Particle_PS.hlsl",
            .VertexLayout = VertexLayoutType::Particle
        });
}

bool MaterialShaderRegistry::Resolve(const MaterialDescription& aDescription, ShaderProgramDescription& outProgram) const
{
    std::string programName;

    switch (aDescription.Type)
    {
        case MaterialType::Surface:
        {
            if (aDescription.ShadingModel == ShadingModel::Unlit)
            {
                programName = "Surface.Unlit";
            }
            else if (aDescription.ShadingModel == ShadingModel::Lit)
            {
                if (aDescription.BlendMode == MaterialBlendMode::Opaque)
                {
                    programName = "Surface.PBR";
                }
                else if (aDescription.BlendMode == MaterialBlendMode::Alpha)
                {
                    programName = "Surface.PBR.Transparent";
                }
            }

            break;
        }

        case MaterialType::Sprite:
            programName = "Sprite.Default";
            break;

        case MaterialType::Font:
            programName = "Font.Default";
            break;

        case MaterialType::Particle:
            programName = "Particle.Default";
            break;

        default:
            return false;
    }

    const auto it = myPrograms.find(programName);

    if (it == myPrograms.end())
    {
        SPDLOG_ERROR("Could not find shader program '{}' for material '{}'", programName, aDescription.Name);
        return false;
    }

    outProgram = it->second;
    return true;
}
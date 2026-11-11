#pragma once

#include <filesystem>

class RenderHardwareInterface;
class MaterialShaderRegistry;
class Material;

struct RHIShaderReflectionInfo;
struct MaterialDescription;
struct ShaderProgramDescription;
struct VertexElementDescription;

enum class VertexLayoutType : uint8_t;

class MaterialFactory
{
public:
    static bool Create(
        const MaterialDescription& aDescription,
        RenderHardwareInterface& aRHI,
        const MaterialShaderRegistry& aShaderRegistry,
        const std::filesystem::path& aShaderRoot,
        Material& outMaterial
    );

private:
    static bool CreateSurfaceMaterial(
        const MaterialDescription& aDescription,
        RenderHardwareInterface& aRHI,
        const MaterialShaderRegistry& aShaderRegistry,
        const std::filesystem::path& aShaderRoot,
        Material& outMaterial);

    static bool CreateSpriteMaterial(
        const MaterialDescription& aDescription,
        RenderHardwareInterface& aRHI,
        const MaterialShaderRegistry& aShaderRegistry,
        const std::filesystem::path& aShaderRoot,
        Material& outMaterial);

    static bool CreateFontMaterial(
        const MaterialDescription& aDescription,
        RenderHardwareInterface& aRHI,
        const MaterialShaderRegistry& aShaderRegistry,
        const std::filesystem::path& aShaderRoot,
        Material& outMaterial);

    static bool CreateParticleMaterial(
        const MaterialDescription& aDescription,
        RenderHardwareInterface& aRHI,
        const MaterialShaderRegistry& aShaderRegistry,
        const std::filesystem::path& aShaderRoot,
        Material& outMaterial);

    static bool CreateFromProgram(
        const MaterialDescription& aDescription,
        const ShaderProgramDescription& aProgram,
        const std::vector<std::pair<std::string, std::string>>& aMacros,
        RenderHardwareInterface& aRHI,
        const std::filesystem::path& aShaderRoot,
        Material& outMaterial);

    static bool GetVertexLayoutDescription(
        VertexLayoutType aType,
        std::vector<VertexElementDescription>& outElements);

    static void InitializeMaterialParameters(
        const RHIShaderReflectionInfo& aVSInfo,
        const RHIShaderReflectionInfo& aPSInfo,
        Material& outMaterial);

    static void CreateMaterialTextureSlots(
        const RHIShaderReflectionInfo& aShaderInfo, 
        Material& inoutMaterial);
};
#pragma once

#include <filesystem>

class RenderHardwareInterface;
class MaterialShaderRegistry;
class Material;

struct RHIShaderReflectionInfo;
struct MaterialDescription;

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
    static void CreateMaterialTextureSlots(const RHIShaderReflectionInfo& aShaderInfo, Material& inoutMaterial);
};
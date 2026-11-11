#pragma once

#include <memory>

class Material;
class Texture;
class RenderHardwareInterface;
class GraphicsEngine;

enum class MaterialType : uint8_t;
enum class MaterialDomain : uint8_t;
enum class ShadingModel : uint8_t;
enum class MaterialBlendMode : uint8_t;
enum class MaterialLayout : uint8_t;

class BuiltinAssets
{
public:
    bool Initialize(GraphicsEngine& aGraphicsEngine, RenderHardwareInterface& aRHI);

    const Material* GetDefaultMaterial(MaterialType aType) const;

private:
    bool CreateDefaultMaterial(
		GraphicsEngine& aGraphicsEngine,
		MaterialType aType,
		const char* aName,
		MaterialDomain aDomain,
		ShadingModel aShadingModel,
		MaterialBlendMode aBlendMode,
		MaterialLayout aLayout);

    std::array<std::unique_ptr<Material>, 5> myDefaultMaterials;

    std::shared_ptr<Texture> myDefaultAlbedoTexture;
    std::shared_ptr<Texture> myDefaultNormalTexture;
    std::shared_ptr<Texture> myDefaultEmissionTexture;
};
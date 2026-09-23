#pragma once

#include <memory>
#include <filesystem>

class Material;
class Texture;
class RenderHardwareInterface;
class GraphicsEngine;

class BuiltinAssets
{
public:
    bool Initialize(GraphicsEngine& aGraphicsEngine, RenderHardwareInterface& aRHI, const std::filesystem::path& aShaderRoot);

    const Material& GetDefaultMaterial() const;
    const Material& GetDefaultFontMaterial() const;
    const Texture& GetDefaultAlbedoTexture() const;
    const Texture& GetDefaultNormalTexture() const;

private:
    std::unique_ptr<Material> myDefaultMaterial;
    std::unique_ptr<Material> myDefaultFontMaterial;
    std::shared_ptr<Texture> myDefaultAlbedoTexture;
    std::shared_ptr<Texture> myDefaultNormalTexture;
};
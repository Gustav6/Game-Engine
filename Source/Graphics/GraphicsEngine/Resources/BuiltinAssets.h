#pragma once

#include <memory>

class Material;
class Texture;
class RenderHardwareInterface;
class GraphicsEngine;

class BuiltinAssets
{
public:
    bool Initialize(GraphicsEngine& aGraphicsEngine, RenderHardwareInterface& aRHI);

    const Material& GetDefaultMaterial() const;
    const Material& GetDefaultSpriteMaterial() const;
    const Texture& GetDefaultAlbedoTexture() const;
    const Texture& GetDefaultNormalTexture() const;
    const Texture& GetDefaultEmissionTexture() const;

private:
    std::unique_ptr<Material> myDefaultMaterial;
    std::unique_ptr<Material> myDefaultSpriteMaterial;
    std::shared_ptr<Texture> myDefaultAlbedoTexture;
    std::shared_ptr<Texture> myDefaultNormalTexture;
    std::shared_ptr<Texture> myDefaultEmissionTexture;
};
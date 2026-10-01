#include "GraphicsEngine.pch.h"
#include "BuiltinAssets.h"

#include "GraphicsEngine/GraphicsEngine.hpp"

#include "GraphicsEngine/Objects/GPU/Texture.h"

#include "GraphicsEngine/RHI/RenderHardwareInterface.h"

#include "GraphicsEngine/Materials/Material.h"

#include "Textures/Albedo/Default_C.h"
#include "Textures/Normal/Default_N.h"
#include "Textures/Effects Map/Default_FX.h"

bool BuiltinAssets::Initialize(GraphicsEngine& aGraphicsEngine, RenderHardwareInterface& aRHI, const std::filesystem::path& aShaderRoot)
{
	// Mesh Material
	{
		myDefaultMaterial = std::make_unique<Material>();

		MaterialDescription defaultMatDesc;
		defaultMatDesc.Name = "Default";
		defaultMatDesc.Domain = MaterialDomain::Surface;
		defaultMatDesc.ShadingModel = ShadingModel::Unlit;
		defaultMatDesc.BlendMode = MaterialBlendMode::Opaque;
		defaultMatDesc.MaterialShaderCode = aShaderRoot / "Material" / "Include" / "Material.hlsli";

		if (!aGraphicsEngine.CreateMaterial(defaultMatDesc, *myDefaultMaterial))
		{
			GELOG(Error, "Failed to create default material!");
			return false;
		}
	}

	// Font Material
	{
		myDefaultFontMaterial = std::make_unique<Material>();

		MaterialDescription fontMaterialDesc;
		fontMaterialDesc.Name = "FontMaterial";
		fontMaterialDesc.Domain = MaterialDomain::Surface;
		fontMaterialDesc.ShadingModel = ShadingModel::Unlit;
		fontMaterialDesc.BlendMode = MaterialBlendMode::Alpha;

		if (!aGraphicsEngine.CreateMaterial(fontMaterialDesc, *myDefaultFontMaterial))
		{
			GELOG(Error, "Failed to create font material!");
			return false;
		}
	}

	// Sprite Material
	{
		myDefaultSpriteMaterial = std::make_unique<Material>();

		MaterialDescription spriteMaterialDesc;
		spriteMaterialDesc.Name = "SpriteMaterial";
		spriteMaterialDesc.Domain = MaterialDomain::Surface;
		spriteMaterialDesc.ShadingModel = ShadingModel::Unlit;
		spriteMaterialDesc.BlendMode = MaterialBlendMode::Alpha;

		if (!aGraphicsEngine.CreateMaterial(spriteMaterialDesc, *myDefaultSpriteMaterial))
		{
			GELOG(Error, "Failed to create sprite material!");
			return false;
		}
	}

	// Albedo Texture
	{
		myDefaultAlbedoTexture = std::make_shared<Texture>();

		if (!aRHI.CreateTexture("Default Albedo", BuiltIn_Default_C_ByteCode, sizeof(BuiltIn_Default_C_ByteCode), *myDefaultAlbedoTexture))
		{
			GELOG(Error, "Failed to create default albedo texture!");
			return false;
		}

		myDefaultMaterial->SetTexture(0, myDefaultAlbedoTexture);
	}

	// Normal Texture
	{
		myDefaultNormalTexture = std::make_shared<Texture>();

		if (!aRHI.CreateTexture("Default Normal", BuiltIn_Default_N_ByteCode, sizeof(BuiltIn_Default_N_ByteCode), *myDefaultNormalTexture))
		{
			GELOG(Error, "Failed to create default normal texture!");
			return false;
		}

		myDefaultMaterial->SetTexture(1, myDefaultNormalTexture);
	}

	// Normal Texture
	{
		myDefaultEmissionTexture = std::make_shared<Texture>();

		if (!aRHI.CreateTexture("Default Emission", BuiltIn_Default_FX_ByteCode, sizeof(BuiltIn_Default_FX_ByteCode), *myDefaultEmissionTexture))
		{
			GELOG(Error, "Failed to create default Emission texture!");
			return false;
		}

		myDefaultMaterial->SetTexture(3, myDefaultEmissionTexture);
	}

	return true;
}

const Material& BuiltinAssets::GetDefaultMaterial() const
{
	return *myDefaultMaterial;
}

const Material& BuiltinAssets::GetDefaultFontMaterial() const
{
	return *myDefaultFontMaterial;
}

const Material& BuiltinAssets::GetDefaultSpriteMaterial() const
{
	return *myDefaultSpriteMaterial;
}

const Texture& BuiltinAssets::GetDefaultAlbedoTexture() const
{
	return *myDefaultAlbedoTexture;
}

const Texture& BuiltinAssets::GetDefaultNormalTexture() const
{
	return *myDefaultNormalTexture;
}

const Texture& BuiltinAssets::GetDefaultEmissionTexture() const
{
	return *myDefaultEmissionTexture;
}

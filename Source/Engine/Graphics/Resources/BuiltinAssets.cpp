#include "GraphicsEngine.pch.h"
#include "BuiltinAssets.h"

#include "Graphics/GraphicsEngine.hpp"

#include "Graphics/Objects/GPU/Texture.h"

#include "Graphics/RHI/RenderHardwareInterface.h"

#include "Graphics/Materials/Material.h"

#include "Textures/Albedo/Default_C.h"
#include "Textures/Normal/Default_N.h"
#include "Textures/Effects Map/Default_FX.h"

bool BuiltinAssets::Initialize(GraphicsEngine& aGraphicsEngine, RenderHardwareInterface& aRHI)
{
	// Mesh Material
	{
		myDefaultMaterial = std::make_unique<Material>();

		MaterialDescription defaultMatDesc;
		defaultMatDesc.Name = "Default_Material";
		defaultMatDesc.Domain = MaterialDomain::Surface;
		defaultMatDesc.ShadingModel = ShadingModel::Unlit;
		defaultMatDesc.BlendMode = MaterialBlendMode::Opaque;
		defaultMatDesc.Layout = MaterialLayout::PBR;

		if (!aGraphicsEngine.CreateMaterial(defaultMatDesc, *myDefaultMaterial))
		{
			SPDLOG_CRITICAL("Failed to create default mesh material!");
			return false;
		}
	}

	// Albedo Texture
	{
		myDefaultAlbedoTexture = std::make_shared<Texture>();

		if (!aRHI.CreateTexture("Default Albedo", BuiltIn_Default_C_ByteCode, sizeof(BuiltIn_Default_C_ByteCode), *myDefaultAlbedoTexture))
		{
			SPDLOG_CRITICAL("Failed to create default albedo texture!");
			return false;
		}

		myDefaultMaterial->SetTexture(0, myDefaultAlbedoTexture);
	}

	// Normal Texture
	{
		myDefaultNormalTexture = std::make_shared<Texture>();

		if (!aRHI.CreateTexture("Default Normal", BuiltIn_Default_N_ByteCode, sizeof(BuiltIn_Default_N_ByteCode), *myDefaultNormalTexture))
		{
			SPDLOG_CRITICAL("Failed to create default normal texture!");
			return false;
		}

		myDefaultMaterial->SetTexture(1, myDefaultNormalTexture);
	}

	// Normal Texture
	{
		myDefaultEmissionTexture = std::make_shared<Texture>();

		if (!aRHI.CreateTexture("Default Emission", BuiltIn_Default_FX_ByteCode, sizeof(BuiltIn_Default_FX_ByteCode), *myDefaultEmissionTexture))
		{
			SPDLOG_CRITICAL("Failed to create default Emission texture!");
			return false;
		}

		myDefaultMaterial->SetTexture(3, myDefaultEmissionTexture);
	}

	return true;
}

const Material* BuiltinAssets::GetDefaultMaterial() const
{
	return myDefaultMaterial.get();
}

const Texture* BuiltinAssets::GetDefaultAlbedoTexture() const
{
	return myDefaultAlbedoTexture.get();
}

const Texture* BuiltinAssets::GetDefaultNormalTexture() const
{
	return myDefaultNormalTexture.get();
}

const Texture* BuiltinAssets::GetDefaultEmissionTexture() const
{
	return myDefaultEmissionTexture.get();
}

#include "GraphicsEngine.pch.h"
#include "BuiltinAssets.h"

#include "GraphicsEngine/GraphicsEngine.hpp"

#include "GraphicsEngine/Objects/GPU/Texture.h"

#include "GraphicsEngine/RHI/RenderHardwareInterface.h"

#include "GraphicsEngine/Materials/Material.h"

#include "Textures/Albedo/Default_C.h"
#include "Textures/Normal/Default_N.h"
#include "Textures/Effects Map/Default_FX.h"

bool BuiltinAssets::Initialize(GraphicsEngine& aGraphicsEngine, RenderHardwareInterface& aRHI)
{
	// Mesh Material
	{
		myDefaultMaterial = std::make_unique<Material>();

		MaterialDescription defaultMatDesc;
		defaultMatDesc.Name = "Default Mesh Material";
		defaultMatDesc.Domain = MaterialDomain::Surface;
		defaultMatDesc.ShadingModel = ShadingModel::Unlit;
		defaultMatDesc.BlendMode = MaterialBlendMode::Opaque;

		if (!aGraphicsEngine.CreateMaterial(defaultMatDesc, *myDefaultMaterial))
		{
			GELOG(Error, "Failed to create default mesh material!");
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
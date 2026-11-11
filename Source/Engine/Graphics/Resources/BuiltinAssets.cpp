#include "Graphics/GraphicsEngine.pch.h"
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
	// Surface
	if (!CreateDefaultMaterial(
		aGraphicsEngine,
		MaterialType::Surface,
		"Default_Surface",
		MaterialDomain::Surface,
		ShadingModel::Lit,
		MaterialBlendMode::Opaque,
		MaterialLayout::PBR))
	{
		return false;
	}

	//// Sprite
	//if (!CreateDefaultMaterial(
	//	aGraphicsEngine,
	//	MaterialType::Sprite,
	//	"Default_Sprite",
	//	MaterialDomain::None,
	//	ShadingModel::None,
	//	MaterialBlendMode::Alpha,
	//	MaterialLayout::None))
	//{
	//	return false;
	//}

	//// Font
	//if (!CreateDefaultMaterial(
	//	aGraphicsEngine,
	//	MaterialType::Font,
	//	"Default_Font",
	//	MaterialDomain::None,
	//	ShadingModel::None,
	//	MaterialBlendMode::Alpha,
	//	MaterialLayout::None))
	//{
	//	return false;
	//}

	//// Particle
	//if (!CreateDefaultMaterial(
	//	aGraphicsEngine,
	//	MaterialType::Particle,
	//	"Default_Particle",
	//	MaterialDomain::None,
	//	ShadingModel::None,
	//	MaterialBlendMode::Alpha,
	//	MaterialLayout::None))
	//{
	//	return false;
	//}

	// Line
	//if (!CreateDefaultMaterial(
	//	aGraphicsEngine,
	//	MaterialType::Line,
	//	"Default_Line",
	//	MaterialDomain::None,
	//	ShadingModel::None,
	//	MaterialBlendMode::Opaque,
	//	MaterialLayout::None))
	//{
	//	return false;
	//}

	// Albedo Texture
	{
		myDefaultAlbedoTexture = std::make_shared<Texture>();

		if (!aRHI.CreateTexture("Default Albedo", BuiltIn_Default_C_ByteCode, sizeof(BuiltIn_Default_C_ByteCode), *myDefaultAlbedoTexture))
		{
			SPDLOG_CRITICAL("Failed to create default albedo texture!");
			return false;
		}

		myDefaultMaterials[static_cast<size_t>(MaterialType::Surface) - 1]->SetTexture(0, myDefaultAlbedoTexture);
	}

	// Normal Texture
	{
		myDefaultNormalTexture = std::make_shared<Texture>();

		if (!aRHI.CreateTexture("Default Normal", BuiltIn_Default_N_ByteCode, sizeof(BuiltIn_Default_N_ByteCode), *myDefaultNormalTexture))
		{
			SPDLOG_CRITICAL("Failed to create default normal texture!");
			return false;
		}

		myDefaultMaterials[static_cast<size_t>(MaterialType::Surface) - 1]->SetTexture(1, myDefaultNormalTexture);
	}

	// Normal Texture
	{
		myDefaultEmissionTexture = std::make_shared<Texture>();

		if (!aRHI.CreateTexture("Default Emission", BuiltIn_Default_FX_ByteCode, sizeof(BuiltIn_Default_FX_ByteCode), *myDefaultEmissionTexture))
		{
			SPDLOG_CRITICAL("Failed to create default Emission texture!");
			return false;
		}

		myDefaultMaterials[static_cast<size_t>(MaterialType::Surface) - 1]->SetTexture(3, myDefaultEmissionTexture);
	}

	return true;
}

const Material* BuiltinAssets::GetDefaultMaterial(MaterialType aType) const
{
	const int index = static_cast<int>(aType);

	if (index == 0 || index > myDefaultMaterials.size())
	{
		return nullptr;
	}

	return myDefaultMaterials[index - 1].get();
}

bool BuiltinAssets::CreateDefaultMaterial(
	GraphicsEngine& aGraphicsEngine,
	MaterialType aType,
	const char* aName,
	MaterialDomain aDomain,
	ShadingModel aShadingModel,
	MaterialBlendMode aBlendMode,
	MaterialLayout aLayout)
{
	const size_t index = static_cast<size_t>(aType) - 1;

	myDefaultMaterials[index] = std::make_unique<Material>();

	MaterialDescription desc;
	desc.Name = aName;
	desc.Type = aType;
	desc.Domain = aDomain;
	desc.ShadingModel = aShadingModel;
	desc.BlendMode = aBlendMode;
	desc.Layout = aLayout;

	if (!aGraphicsEngine.CreateMaterial(desc, *myDefaultMaterials[index]))
	{
		SPDLOG_CRITICAL("Failed to create default material [{}]!", aName);
		return false;
	}

	return true;
}

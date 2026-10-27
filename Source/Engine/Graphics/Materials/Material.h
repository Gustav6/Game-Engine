#pragma once

#include <filesystem>

#include "MaterialInterface.h"
#include "Graphics/RHI/PipelineStateObject.h"

#pragma warning(push)
#pragma warning(disable : 4324)

struct MaterialDescription
{
	std::string Name;

	MaterialDomain Domain = MaterialDomain::None;
	ShadingModel ShadingModel = ShadingModel::None;
	MaterialBlendMode BlendMode = MaterialBlendMode::None;
	MaterialLayout Layout = MaterialLayout::None;
	MaterialFeatureFlags Features = FeatureFlag_None;

	std::filesystem::path MaterialShaderCode;
};

class Material : public MaterialInterface
{
	friend class GraphicsEngine;
	friend class MaterialFactory;

public:
	std::string_view GetName() const override { return myName; }
	MaterialDomain GetDomain() const override { return myDescription.Domain; }
	ShadingModel GetShadingModel() const override { return myDescription.ShadingModel; }
	MaterialBlendMode GetBlendMode() const override { return myDescription.BlendMode; }
	MaterialLayout GetLayout() const override { return myDescription.Layout; }
	MaterialFeatureFlags GetFeatures() const override { return myDescription.Features; }

	const PipelineStateObject& GetPSO() const override { return myPSO; }
	
	const uint8_t* GetParameterDataBlock() const override { return myData; };
	bool HasParameters() const override { return !myParameters.empty(); };
	const std::vector<MaterialParameterInfo>& GetParameters() const override { return myParameters; };

	bool IsMaterialDataDirty() const override;
	void RefreshMaterialData() const override;
	const MaterialParameterInfo* GetParameterByIndex(unsigned aIndex) const override;
	const MaterialParameterInfo* GetParameterByName(const std::string& aName) const override;

	unsigned int GetTextureSlotByName(const std::string& aName) const override;

	bool SetTexture(const std::string& aName, const std::shared_ptr<Texture>& aTexture) override;
	bool SetTexture(unsigned int aSlot, const std::shared_ptr<Texture>& aTexture) override;

	std::shared_ptr<Texture> GetTexture(const std::string aName) const override;
	std::shared_ptr<Texture> GetTexture(unsigned int aSlot) const override;

protected:
	bool SetRawParameterValue(const MaterialParameterInfo& aParamInfo, const void* aPtr, size_t aPtrSize) override;

private:
	alignas(16) uint8_t myData[MATERIAL_BUFFER_SIZE] = {};
	std::vector<MaterialParameterInfo> myParameters;
	std::unordered_map<std::string, unsigned int> myParameterNameToIndex;

	std::shared_ptr<Texture> myTextures[MAX_MATERIAL_TEXTURE_COUNT] = {};
	std::unordered_map<std::string, unsigned int> myTexturesSlotNameToIndex;

	MaterialDescription myDescription;
	PipelineStateObject myPSO;
	std::string myName;
};

class MaterialInstance : public MaterialInterface
{
public:
	static std::shared_ptr<MaterialInstance> Create(std::string_view aName, const std::shared_ptr<MaterialInterface>& aMaterialInterface);

	std::string_view GetName() const override { return myName; }
	MaterialDomain GetDomain() const override { return myParentMaterial->GetDomain(); }
	ShadingModel GetShadingModel() const override { return myParentMaterial->GetShadingModel(); }
	MaterialBlendMode GetBlendMode() const override { return myParentMaterial->GetBlendMode(); }
	MaterialLayout GetLayout() const override { return myParentMaterial->GetLayout(); }
	MaterialFeatureFlags GetFeatures() const override { return myParentMaterial->GetFeatures(); }

	const PipelineStateObject& GetPSO() const override { return myParentMaterial->GetPSO(); }
	
	const uint8_t* GetParameterDataBlock() const override { return myData; };
	bool HasParameters() const override { return myParentMaterial->HasParameters(); };
	const std::vector<MaterialParameterInfo>& GetParameters() const override { return myParentMaterial->GetParameters(); };

	bool IsMaterialDataDirty() const override;
	void RefreshMaterialData() const override;
	const MaterialParameterInfo* GetParameterByIndex(unsigned aIndex) const override { return myParentMaterial->GetParameterByIndex(aIndex); };
	const MaterialParameterInfo* GetParameterByName(const std::string& aName) const override { return myParentMaterial->GetParameterByName(aName); };

	unsigned int GetTextureSlotByName(const std::string& aName) const override;

	bool SetTexture(const std::string& aName, const std::shared_ptr<Texture>& aTexture) override;
	bool SetTexture(unsigned int aSlot, const std::shared_ptr<Texture>& aTexture) override;

	std::shared_ptr<Texture> GetTexture(const std::string aName) const override;
	std::shared_ptr<Texture> GetTexture(unsigned int aSlot) const override;

protected:
	bool SetRawParameterValue(const MaterialParameterInfo& aParamInfo, const void* aPtr, size_t aPtrSize) override;

private:
	alignas(16) mutable uint8_t myData[MATERIAL_BUFFER_SIZE] = {};

	std::optional<std::shared_ptr<Texture>> myTextures[MAX_MATERIAL_TEXTURE_COUNT] = {};

	std::string myName;

	std::shared_ptr<const MaterialInterface> myParentMaterial;

	mutable bool myIsParameterDataDirty = false;

	std::vector<bool> myOverriddenParameters;
};

#pragma warning(pop)
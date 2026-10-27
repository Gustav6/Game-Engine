#include "GraphicsEngine.pch.h"
#include "Material.h"

bool Material::IsMaterialDataDirty() const
{
    return false;
}

void Material::RefreshMaterialData() const
{
    // Nothing here for Material.
}

const MaterialParameterInfo* Material::GetParameterByIndex(unsigned aIndex) const
{
    if (myParameters.size() <= aIndex)
    {
        return nullptr;
    }

    return &myParameters.at(aIndex);
}

const MaterialParameterInfo* Material::GetParameterByName(const std::string& aName) const
{
    const auto it = myParameterNameToIndex.find(aName);

    if (it == myParameterNameToIndex.end())
    {
        return nullptr;
    }

    return &myParameters[it->second];
}

unsigned int Material::GetTextureSlotByName(const std::string& aName) const
{
    std::string lowerName = aName;
    std::ranges::transform(lowerName, lowerName.begin(), tolower);

    const auto it = myTexturesSlotNameToIndex.find(lowerName);

    if (it == myTexturesSlotNameToIndex.end())
    {
        return std::numeric_limits<unsigned int>::max();
    }

    return it->second;
}

bool Material::SetTexture(const std::string& aName, const std::shared_ptr<Texture>& aTexture)
{
    const unsigned slot = GetTextureSlotByName(aName);

    return SetTexture(slot, aTexture);
}

bool Material::SetTexture(unsigned int aSlot, const std::shared_ptr<Texture>& aTexture)
{
    if (aSlot > MAX_MATERIAL_TEXTURE_COUNT)
    {
        return false;
    }

    myTextures[aSlot] = aTexture;

    return true;
}

std::shared_ptr<Texture> Material::GetTexture(const std::string aName) const
{
    const unsigned slot = GetTextureSlotByName(aName);

    return GetTexture(slot);
}

std::shared_ptr<Texture> Material::GetTexture(unsigned int aSlot) const
{
    if (aSlot > MAX_MATERIAL_TEXTURE_COUNT)
    {
        return nullptr;
    }

    return myTextures[aSlot];
}

bool Material::SetRawParameterValue(const MaterialParameterInfo& aParamInfo, const void* aPtr, size_t aPtrSize)
{
    if (aParamInfo.Offset > MATERIAL_BUFFER_SIZE ||
        aParamInfo.Offset + aParamInfo.Size > MATERIAL_BUFFER_SIZE ||
        aParamInfo.Size > aPtrSize)
    {
        return false;
    }

    memcpy_s(myData + aParamInfo.Offset, aParamInfo.Size, aPtr, aParamInfo.Size);

    return true;
}

std::shared_ptr<MaterialInstance> MaterialInstance::Create(std::string_view aName, const std::shared_ptr<MaterialInterface>& aMaterialInterface)
{
    std::shared_ptr<MaterialInstance> instance = std::make_shared<MaterialInstance>();
    instance->myParentMaterial = aMaterialInterface;

    memcpy_s(instance->myData, MATERIAL_BUFFER_SIZE, aMaterialInterface->GetParameterDataBlock(), MATERIAL_BUFFER_SIZE);

    instance->myOverriddenParameters.resize(aMaterialInterface->GetParameters().size());
    instance->myIsParameterDataDirty = true;
    instance->myName = aName;

    return instance;
}

bool MaterialInstance::IsMaterialDataDirty() const
{
    return myIsParameterDataDirty || myParentMaterial->IsMaterialDataDirty();
}

void MaterialInstance::RefreshMaterialData() const
{
    if (myParentMaterial->IsMaterialDataDirty())
    {
        // Cache settings from parent.

        std::vector<MaterialParameterInfo> overriddenParams;
        overriddenParams.reserve(myOverriddenParameters.size());

        for (size_t i = 0; i < myOverriddenParameters.size(); i++)
        {
            if (!myOverriddenParameters[i])
            {
                continue;
            }

            overriddenParams.emplace_back(*GetParameterByIndex(static_cast<unsigned int>(i)));
        }

        alignas(16) uint8_t settingsCache[MATERIAL_BUFFER_SIZE] = {};
        memcpy_s(settingsCache, MATERIAL_BUFFER_SIZE, myData, MATERIAL_BUFFER_SIZE);

        myParentMaterial->RefreshMaterialData();

        memcpy_s(myData, MATERIAL_BUFFER_SIZE, myParentMaterial->GetParameterDataBlock(), MATERIAL_BUFFER_SIZE);

        for (const auto& overriddenParam : overriddenParams)
        {
            if (const MaterialParameterInfo* info = GetParameterByName(overriddenParam.Name))
            {
                uint8_t* srcPtr = settingsCache + overriddenParam.Offset;
                uint8_t* dstPtr = myData + overriddenParam.Offset;
                memcpy_s(dstPtr, info->Size, srcPtr, overriddenParam.Size);
            }
        }
    }

    myIsParameterDataDirty = false;
}

unsigned int MaterialInstance::GetTextureSlotByName(const std::string& aName) const
{
    return myParentMaterial->GetTextureSlotByName(aName);
}

bool MaterialInstance::SetTexture(const std::string& aName, const std::shared_ptr<Texture>& aTexture)
{
    const unsigned int slot = GetTextureSlotByName(aName);

    return SetTexture(slot, aTexture);
}

bool MaterialInstance::SetTexture(unsigned int aSlot, const std::shared_ptr<Texture>& aTexture)
{
    if (aSlot > MAX_MATERIAL_TEXTURE_COUNT)
    {
        return false;
    }

    myTextures[aSlot] = aTexture;

    return true;
}

std::shared_ptr<Texture> MaterialInstance::GetTexture(const std::string aName) const
{
    const unsigned int slot = GetTextureSlotByName(aName);

    return GetTexture(slot);
}

std::shared_ptr<Texture> MaterialInstance::GetTexture(unsigned int aSlot) const
{
    if (aSlot > MAX_MATERIAL_TEXTURE_COUNT)
    {
        return nullptr;
    }

    if (!myTextures[aSlot].has_value())
    {
        return myParentMaterial->GetTexture(aSlot);
    }

    return myTextures[aSlot].value();
}

bool MaterialInstance::SetRawParameterValue(const MaterialParameterInfo& aParamInfo, const void* aPtr, size_t aPtrSize)
{
    if (aParamInfo.Offset > MATERIAL_BUFFER_SIZE || 
        aParamInfo.Offset + aParamInfo.Size > MATERIAL_BUFFER_SIZE || 
        aParamInfo.Size > aPtrSize)
    {
        return false;
    }

    memcpy_s(myData + aParamInfo.Offset, aParamInfo.Size, aPtr, aParamInfo.Size);

    myIsParameterDataDirty = true;
    myOverriddenParameters[aParamInfo.Index] = true;

    return true;
}

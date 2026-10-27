#include "Core.pch.h"
#include "MaterialAsset.h"

#include "Graphics/GraphicsEngine.hpp"
#include "Graphics/Materials/Material.h"

#include "Core/Assets/Registry/AssetRegistry.h"

#include "TextureAsset.h"

enum class ParameterType
{
    ScalarFloat = 0,
    Vector3Float = 1,
    Vector3Double = 2,
	Texture = 3,
    TextureCollection = 4,
    Font = 5,
    RuntimeVirtualTexture = 6,
    SparseVolumeTexture = 7,
    StaticSwitch = 8,
    ParameterCollection = 9
};

const std::shared_ptr<MaterialInterface>& MaterialAsset::GetMaterial() const
{
    return myMaterial;
}

bool MaterialAsset::Load(const std::filesystem::path& aPath)
{
    Json::Data materialData = Json::Read(aPath);

    if (!materialData.IsValid())
    {
        SPDLOG_ERROR("Failed to read material JSON [{}]", aPath.string());
        return false;
    }

    if (materialData.HasKey("Parent"))
    {
        return LoadUnrealInstanceData(materialData);
    }

    return LoadUnrealMasterData(materialData);
}

bool MaterialAsset::Construct(const Json::Data& aMaterialData, MaterialType)
{
    if (!aMaterialData.IsValid())
    {
        SPDLOG_ERROR("Cannot construct material from invalid JSON data!");
        return false;
    }

    if (aMaterialData.HasKey("Parent"))
    {
        return LoadUnrealInstanceData(aMaterialData);
    }

    return LoadUnrealMasterData(aMaterialData);
}

bool MaterialAsset::LoadUnrealInstanceData(const Json::Data& aMaterialData)
{
    bool hasErrord = false;

    std::string materialName = aMaterialData.GetString("Name");

    if (materialName.empty())
    {
        SPDLOG_WARN("Constructed material has no name, using placeholder");
        materialName = "NO NAME GIVEN";
        hasErrord = true;
    }

    std::string parent = aMaterialData.GetString("Parent");

    if (parent == "Self")
    {
        parent = aMaterialData.GetString("Name");
    }

    std::shared_ptr<MaterialAsset> masterMaterialAsset = AssetRegistry::Get().GetAsset<MaterialAsset>(parent);

    if (masterMaterialAsset == nullptr || masterMaterialAsset->GetMaterial() == nullptr)
    {
        SPDLOG_ERROR("Material [{}] failed to get master material [{}]!", materialName, parent);
        hasErrord = true;
    }
    else
    {
        std::shared_ptr<MaterialInstance> instance = MaterialInstance::Create(materialName, masterMaterialAsset->GetMaterial());

        std::vector<Json::Data> parameters = aMaterialData.GetDataObjectArray("Parameters");

        if (parameters.empty())
        {
            SPDLOG_WARN("Material has no Parameters [{}]!", materialName);
        }

        PushParameters(parameters, instance.get());

        myMaterial = std::move(instance);
    }

    return !hasErrord;
}

bool MaterialAsset::LoadUnrealMasterData(const Json::Data& aMaterialData)
{
    bool hasErrord = false;

    std::string materialName = aMaterialData.GetString("Name");

    if (materialName.empty())
    {
        SPDLOG_WARN("Constructed material has no name, using placeholder");
        materialName = "NO NAME GIVEN";
        hasErrord = true;
    }

    std::vector<Json::Data> parameters = aMaterialData.GetDataObjectArray("Parameters");

    if (parameters.empty())
    {
        SPDLOG_WARN("Material has no Parameters [{}]!", materialName);
    }

    MaterialDescription desc;
    desc.Name = materialName;
    desc.Layout = MaterialLayout::PBR;

    for (const auto& parameter : parameters)
    {
        const std::string parameterName = parameter.GetString("Name");
        const int type = parameter.GetInt("Type");

        switch (static_cast<ParameterType>(type))
        {
            case ParameterType::Texture:
            {
                std::string parameterNameLower = parameterName;
                std::ranges::transform(parameterNameLower, parameterNameLower.begin(), tolower);

                if (parameterNameLower == "splatmap")
                {
                    desc.Layout = MaterialLayout::Splat;
                    break;
                }
                if (parameterNameLower == "Emission")
                {
                    desc.Features |= MaterialFeatureFlag_Emission;
                    break;
                }

                const Json::Data value = parameter.GetDataObject("Value");

                if (value.HasKey("Name"))
                {
                    const std::string textureName = value.GetString("Name");

                    std::string suffix = {};

                    size_t pos = textureName.rfind('_');

                    if (pos == std::string::npos)
                    {
                        continue;
                    }

                    suffix = textureName.substr(pos + 1);
                    std::ranges::transform(suffix, suffix.begin(), tolower);

                    if (suffix == "spm")
                    {
                        desc.Layout = MaterialLayout::Splat;
                        break;
                    }
                    if (suffix == "fx")
                    {
                        desc.Features |= MaterialFeatureFlag_Emission;
                        break;
                    }
                }

                break;
            }
            default:
            {
                break;
            }
        }
    }

    {
        if (aMaterialData.HasKey("Domain"))
        {
            desc.Domain = static_cast<MaterialDomain>(aMaterialData.GetInt("Domain") + 1);
        }
        else
        {
            SPDLOG_ERROR("Material has no Domain [{}]!", materialName);
            hasErrord = true;
        }

        if (aMaterialData.HasKey("ShadingModel"))
        {
            desc.ShadingModel = static_cast<ShadingModel>(aMaterialData.GetInt("ShadingModel") + 1);
        }
        else
        {
            SPDLOG_ERROR("Material has no Shading Model [{}]!", materialName);
            hasErrord = true;
        }

        if (aMaterialData.HasKey("BlendMode"))
        {
            desc.BlendMode = static_cast<MaterialBlendMode>(aMaterialData.GetInt("BlendMode") + 1);
        }
        else
        {
            SPDLOG_ERROR("Material has no Blend Mode [{}]!", materialName);
            hasErrord = true;
        }
    }

    // TODO: Add the ability to choose material code.
    //desc.MaterialShaderCode = AssetRegistry::Get().GetWorkingDirectory() / "Shaders/BaseMaterial.hlsli";

    std::shared_ptr<Material> material = std::make_shared<Material>();

    if (!GraphicsEngine::Get().CreateMaterial(desc, *material))
    {
        SPDLOG_ERROR("Failed to create material [{}]!", materialName);
        hasErrord = true;
    }

    PushParameters(parameters, material.get());

    myMaterial = std::move(material);

    return !hasErrord;
}

void MaterialAsset::PushParameters(const std::vector<Json::Data>& aMaterialParameters, MaterialInterface* aMaterialInstance)
{
    std::string parameterName = {};
    int type = 0;

	for (const auto& parameter : aMaterialParameters)
	{
		parameterName = parameter.GetString("Name");
        type = parameter.GetInt("Type");

		switch (static_cast<ParameterType>(type))
		{
            case ParameterType::ScalarFloat:
            {
                aMaterialInstance->SetValue(parameterName, parameter.GetFloat("Value"));
                break;
            }
            case ParameterType::Vector3Float:
            {
                std::vector<float> values = parameter.GetFloatArray("Value");

                aMaterialInstance->SetValue(parameterName, CommonUtilities::Vector3f(values.data()));
                break;
            }
			case ParameterType::Texture:
			{   
				Json::Data parameterData = parameter.GetDataObject("Value");

                if (!parameterData.HasKey("Name"))
                {
                    SPDLOG_WARN("Texture parameter [{}] has no texture Name, skipping!", parameterName);
                    break;
                }

                std::string textureName = parameterData.GetString("Name");

                std::shared_ptr<TextureAsset> textureAsset = AssetRegistry::Get().GetAsset<TextureAsset>(textureName);

                if (textureAsset == nullptr)
                {
                    SPDLOG_WARN("Texture parameter [{}] could not load texture [{}]!", parameterName, textureName);
                    break;
                }

                myPersistentTextureList.emplace_back(textureAsset);

                const unsigned int slot = aMaterialInstance->GetTextureSlotByName(parameterName);

                if (slot != std::numeric_limits<unsigned int>::max())
                {
                    aMaterialInstance->SetTexture(slot, textureAsset->GetTexture());
                }
                else
                {
                    SPDLOG_WARN("No shader texture slot found for parameter [{}], for [{}]", parameterName, aMaterialInstance->GetName());
                }
                
				break;
			}
            case ParameterType::StaticSwitch:
            {
                SPDLOG_ERROR("STATIC SWITCH [{}]", parameterName);
                break;
            }
			default:
			{
                SPDLOG_ERROR("Non handled parameter type found [{}], Parameter [{}]!", type, parameterName);
				break;
			}
		}
	}
}

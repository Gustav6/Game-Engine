#include "GameFramework.pch.h"
#include "MaterialAsset.h"

#include "GraphicsEngine/GraphicsEngine.hpp"
#include "GraphicsEngine/Materials/Material.h"

#include "AssetPipeline/AssetRegistry/AssetRegistry.h"
#include "TextureAsset.h"

#include "Json/JsonWrapper.h"

#include "tracy/core/Tracy.hpp"

#include "Logger/Logger.h"

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
    JsonData materialData = Json::Read(aPath);

    if (!materialData.IsValid())
    {
        spdlog::error("Failed to read material JSON [{}]", aPath.string());
        return false;
    }

    if (materialData.HasKey("Parent"))
    {
        return LoadInstanceData(materialData);
    }

    return LoadMasterData(materialData);
}

bool MaterialAsset::Construct(const JsonData& aMaterialData)
{
    if (!aMaterialData.IsValid())
    {
        spdlog::error("Cannot construct material from invalid JSON data!");
        return false;
    }

    if (aMaterialData.HasKey("Parent"))
    {
        return LoadInstanceData(aMaterialData);
    }

    return LoadMasterData(aMaterialData);
}

bool MaterialAsset::LoadInstanceData(const JsonData& aMaterialData)
{
    bool hasErrord = false;

    std::string materialName = aMaterialData.GetString("Name");

    if (materialName.empty())
    {
        spdlog::warn("Constructed material has no name, using placeholder");
        materialName = "NO NAME GIVEN";
        hasErrord = true;
    }

    //LOG_GROUP(LogAssetRegistry, Verbose, "Instance Material");

    std::string parent = aMaterialData.GetString("Parent");

    if (parent == "Self")
    {
        parent = aMaterialData.GetString("Name");
    }

    std::shared_ptr<MaterialAsset> masterMaterialAsset = AssetRegistry::Get().GetAsset<MaterialAsset>(parent);

    if (masterMaterialAsset == nullptr)
    {
        spdlog::error("Material [{}] failed to get master material [{}]!", materialName, parent);
        hasErrord = true;
    }
    else
    {
        std::shared_ptr<MaterialInstance> instance = MaterialInstance::Create(materialName, masterMaterialAsset->GetMaterial());

        //LOG_GROUP(MaterialLog, Verbose, "Instance Parameters");

        if (aMaterialData.HasKey("Parameters"))
        {
            std::vector<JsonData> parameters = aMaterialData.GetObjectValueArray("Parameters");

            if (parameters.empty())
            {
                spdlog::error("Material has no Parameters [{}]!", materialName);
                hasErrord = true;
            }

            PushParameters(parameters, instance.get());
        }
        else
        {
            spdlog::error("Material has no Parameters [{}]!", materialName);
            hasErrord = true;
        }

        myMaterial = std::move(instance);
    }

    return !hasErrord;
}

bool MaterialAsset::LoadMasterData(const JsonData& aMaterialData)
{
    bool hasErrord = false;

    std::string materialName = aMaterialData.GetString("Name");

    if (materialName.empty())
    {
        spdlog::warn("Constructed material has no name, using placeholder");
        materialName = "NO NAME GIVEN";
        hasErrord = true;
    }

    //LOG_GROUP(MaterialLog, Verbose, "Master Material");

    MaterialDescription desc;
    desc.Name = materialName;

    {
        //LOG_GROUP(MaterialLog, Verbose, "Master Settings");

        if (aMaterialData.HasKey("Domain"))
        {
            desc.Domain = static_cast<MaterialDomain>(aMaterialData.GetInt("Domain") + 1);
        }
        else
        {
            spdlog::error("Material has no Domain [{}]!", materialName);
            hasErrord = true;
        }

        if (aMaterialData.HasKey("ShadingModel"))
        {
            desc.ShadingModel = static_cast<ShadingModel>(aMaterialData.GetInt("ShadingModel") + 1);
        }
        else
        {
            spdlog::error("Material has no Shading Model [{}]!", materialName);
            hasErrord = true;
        }

        if (aMaterialData.HasKey("BlendMode"))
        {
            desc.BlendMode = static_cast<MaterialBlendMode>(aMaterialData.GetInt("BlendMode") + 1);
        }
        else
        {
            spdlog::error("Material has no Blend Mode [{}]!", materialName);
            hasErrord = true;
        }
    }

    desc.MaterialShaderCode = AssetRegistry::Get().GetWorkingDirectory() / "Shaders/BaseMaterial.hlsli";

    std::shared_ptr<Material> material = std::make_shared<Material>();

    if (!GraphicsEngine::Get().CreateMaterial(desc, *material))
    {
        spdlog::error("Failed to create material [{}]!", materialName);
        hasErrord = true;
    }
    else
    {
        //LOG_GROUP(MaterialLog, Verbose, "Master Parameters");

        if (aMaterialData.HasKey("Parameters"))
        {
            std::vector<JsonData> parameters = aMaterialData.GetObjectValueArray("Parameters");

            if (parameters.empty())
            {
                spdlog::error("Material has no Parameters [{}]!", materialName);
                hasErrord = true;
            }

            PushParameters(parameters, material.get());
        }
        else
        {
            spdlog::error("Material has no Parameters [{}]!", materialName);
            hasErrord = true;
        }
    }

    myMaterial = std::move(material);

    return !hasErrord;
}

void MaterialAsset::PushParameters(const std::vector<JsonData>& aMaterialParameters, MaterialInterface* aMaterialInstance)
{
	for (const auto& parameter : aMaterialParameters)
	{
		std::string parameterName = parameter.GetString("Name");
        int type = parameter.GetInt("Type");

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
				JsonData parameterData = parameter.GetObjectValue("Value");

                if (!parameterData.HasKey("Name"))
                {
                    spdlog::warn("Texture parameter [{}] has no texture Name, skipping", parameterName);
                    break;
                }

                std::string textureName = parameterData.GetString("Name");

                std::shared_ptr<TextureAsset> textureAsset = AssetRegistry::Get().GetAsset<TextureAsset>(textureName);

                if (textureAsset == nullptr)
                {
                    spdlog::warn("Texture parameter [{}] could not load texture [{}]", parameterName, textureName);
                    break;
                }

                std::string suffix = {};

                size_t pos = textureName.rfind('_');

                if (pos != std::string::npos)
                {
                    suffix = textureName.substr(pos + 1);
                }

                std::string lowerCaseSuffix = suffix;
                std::ranges::transform(lowerCaseSuffix, lowerCaseSuffix.begin(), tolower);

                if (lowerCaseSuffix == "c")
                {
                    aMaterialInstance->SetTexture(0, textureAsset->GetTexture());
                }
                else if (lowerCaseSuffix == "n")
                {
                    aMaterialInstance->SetTexture(1, textureAsset->GetTexture());
                }
                else if (lowerCaseSuffix == "orm")
                {
                    aMaterialInstance->SetTexture(2, textureAsset->GetTexture());
                }
                else if (lowerCaseSuffix == "fx")
                {
                    aMaterialInstance->SetTexture(3, textureAsset->GetTexture());
                }
                else
                {
                    spdlog::warn("Texture [{}] has unknown suffix [{}]!", textureName, suffix);
                }

				break;
			}
			default:
			{
                spdlog::error("Non handled parameter type found [{}], Parameter [{}]!", type, aMaterialInstance->GetName());
				break;
			}
		}
	}
}

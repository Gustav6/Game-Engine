#include "GameFramework.pch.h"
#include "MaterialAsset.h"

#include "GraphicsEngine/GraphicsEngine.hpp"
#include "GraphicsEngine/Materials/Material.h"

#include "AssetPipeline/AssetRegistry/AssetRegistry.h"
#include "TextureAsset.h"

#include "CommonUtilities/Json/JsonWrapper.h"

#include "yyjson/yyjson.h"
#include "tracy/core/Tracy.hpp"

#include "Logger/Logger.h"

DECLARE_LOG_CATEGORY_WITH_NAME(MaterialLog, MATERIAL, Verbose);

DEFINE_LOG_CATEGORY(MaterialLog);

MaterialAsset::~MaterialAsset() = default;

const std::shared_ptr<MaterialInterface> MaterialAsset::GetMaterial() const
{
    return myMaterial;
}

bool MaterialAsset::Load(const std::filesystem::path& aPath)
{
    ZoneScopedN("Load Material");

    JsonData materialData = Json::Read(aPath);

    if (!materialData.IsValid())
    {
        return false;
    }

    std::string materialName = materialData.GetString("Name");

    if (materialName.empty())
    {
        LOG(MaterialLog, Error, "Material has no name [{}]!", aPath.string());

        return false;
    }

    JsonData textureSettings = materialData.GetObjectValue("Textures");

    if (!textureSettings.IsValid())
    {
        LOG(MaterialLog, Error, "Material has no valid textures [{}]!", aPath.string());

        return false;
    }

    myTextureAssets = 
    {
	    nullptr,
        nullptr,
        nullptr,
        nullptr
    };

    if (textureSettings.HasKey("AlbedoTexture"))
    {
        myTextureAssets[0] = (AssetRegistry::Get().GetAsset<TextureAsset>(textureSettings.GetString("AlbedoTexture")));
    }
    if (textureSettings.HasKey("NormalTexture"))
    {
        myTextureAssets[1] = (AssetRegistry::Get().GetAsset<TextureAsset>(textureSettings.GetString("NormalTexture")));
    }
    if (textureSettings.HasKey("MaterialTexture"))
    {
        myTextureAssets[2] = (AssetRegistry::Get().GetAsset<TextureAsset>(textureSettings.GetString("MaterialTexture")));
    }
    if (textureSettings.HasKey("EmissionTexture"))
    {
        myTextureAssets[3] = (AssetRegistry::Get().GetAsset<TextureAsset>(textureSettings.GetString("EmissionTexture")));
    }

    if (materialData.HasKey("MasterMaterial"))
    {
        std::shared_ptr<MaterialAsset> masterMaterialAsset = AssetRegistry::Get().GetAsset<MaterialAsset>(materialData.GetString("MasterMaterial"));

        if (masterMaterialAsset == nullptr)
        {
            LOG(MaterialLog, Error, "Failed to get master material [{}]!", aPath.string());

            return false;
        }

        std::shared_ptr<MaterialInstance> instance = MaterialInstance::Create(materialName, masterMaterialAsset->GetMaterial());

        // Checks if a material wants to effect the connected shader!
        if (materialData.HasKey("Shader"))
        {
            // Will assume that if a json shader object exists there will be parameters

            JsonData shaderSettings = materialData.GetObjectValue("Shader");

            JsonData materialParameters = shaderSettings.GetObjectValue("Parameters");

            LoadParameters(materialParameters, instance.get());
        }

        myMaterial = std::move(instance);
    }
    else
    {
        if (!materialData.HasKey("Settings"))
        {
            LOG(MaterialLog, Error, "Material has no valid settings [{}]!", aPath.string());

            return false;
        }

        JsonData materialSettings = materialData.GetObjectValue("Settings");

        MaterialDescription desc;
        desc.Name = materialName;
        desc.Domain = static_cast<MaterialDomain>(materialSettings.GetInt("MaterialDomain"));
        desc.ShadingModel = static_cast<ShadingModel>(materialSettings.GetInt("ShadingModel"));
        desc.BlendMode = static_cast<MaterialBlendMode>(materialSettings.GetInt("BlendMode"));

        // If no shader settings is provided it will run the default shaders code instead!
        if (materialData.HasKey("Shader"))
        {
            JsonData shaderSettings = materialData.GetObjectValue("Shader");

            // Will assume that if a json shader object exists there will be a directory
            desc.MaterialShaderCode = AssetRegistry::Get().GetWorkingDirectory() / shaderSettings.GetString("Code");
        }

        std::shared_ptr<Material> material = std::make_shared<Material>();

        if (!GraphicsEngine::Get().CreateMaterial(desc, *material))
        {
            LOG(MaterialLog, Error, "Failed to create material [{}]!", aPath.string());

            return false;
        }

        myMaterial = std::move(material);
    }

    if (myTextureAssets[0] != nullptr)
    {
        myMaterial->SetTexture(0, myTextureAssets[0]->GetTexture());
    }
    if (myTextureAssets[1] != nullptr)
    {
        myMaterial->SetTexture(1, myTextureAssets[1]->GetTexture());
    }
    if (myTextureAssets[2] != nullptr)
    {
        myMaterial->SetTexture(2, myTextureAssets[2]->GetTexture());
    }
    if (myTextureAssets[3] != nullptr)
    {
        myMaterial->SetTexture(3, myTextureAssets[3]->GetTexture());
    }

    return true;
}

void MaterialAsset::LoadParameters(const JsonData& aMaterialParameters, MaterialInstance* aMaterialInstance)
{
    for (const auto& parameterName : aMaterialParameters.GetKeys())
    {
        switch (aMaterialParameters.GetType(parameterName.c_str()))
        {
            case JsonValueType::Int:
            {
                aMaterialInstance->SetValue(parameterName, aMaterialParameters.GetInt(parameterName.c_str()));
                break;
            }
            case JsonValueType::Unsigned:
            {
                aMaterialInstance->SetValue(parameterName, aMaterialParameters.GetUnsigned(parameterName.c_str()));
                break;
            }
            case JsonValueType::Float:
            {
                aMaterialInstance->SetValue(parameterName, aMaterialParameters.GetFloat(parameterName.c_str()));
                break;
            }
            case JsonValueType::Array:
            {
                switch (aMaterialParameters.GetArrayType(parameterName.c_str()))
                {
                    case JsonValueType::Int:
                    {
                        std::vector<int> values = aMaterialParameters.GetIntArray(parameterName.c_str());

                        if (values.size() == 2)
                        {
                            aMaterialInstance->SetValue(parameterName, CommonUtilities::Vector2i(values.data()));
                        }
                        else if (values.size() == 3)
                        {
                            aMaterialInstance->SetValue(parameterName, CommonUtilities::Vector3i(values.data()));
                        }
                        else if (values.size() == 4)
                        {
                            aMaterialInstance->SetValue(parameterName, CommonUtilities::Vector4i(values.data()));
                        }
                        else
                        {
                            LOG(MaterialLog, Error, "Array parameter has unhandled / invalid length [{}]!", parameterName);
                        }

                        break;
                    }
                    case JsonValueType::Unsigned:
                    {
                        std::vector<unsigned int> values = aMaterialParameters.GetUnsignedArray(parameterName.c_str());

                        if (values.size() == 2)
                        {
                            aMaterialInstance->SetValue(parameterName, CommonUtilities::Vector2ui(values.data()));
                        }
                        else if (values.size() == 3)
                        {
                            aMaterialInstance->SetValue(parameterName, CommonUtilities::Vector3ui(values.data()));
                        }
                        else if (values.size() == 4)
                        {
                            aMaterialInstance->SetValue(parameterName, CommonUtilities::Vector4ui(values.data()));
                        }
                        else
                        {
                            LOG(MaterialLog, Error, "Array parameter has unhandled / invalid length [{}]!", parameterName);
                        }

                        break;
                    }
                    case JsonValueType::Float:
                    {
                        std::vector<float> values = aMaterialParameters.GetFloatArray(parameterName.c_str());

                        if (values.size() == 2)
                        {
                            aMaterialInstance->SetValue(parameterName, CommonUtilities::Vector2f(values.data()));
                        }
                        else if (values.size() == 3)
                        {
                            aMaterialInstance->SetValue(parameterName, CommonUtilities::Vector3f(values.data()));
                        }
                        else if (values.size() == 4)
                        {
                            aMaterialInstance->SetValue(parameterName, CommonUtilities::Vector4f(values.data()));
                        }
                        else
                        {
                            LOG(MaterialLog, Error, "Array parameter has unhandled / invalid length [{}]!", parameterName);
                        }

                        break;
                    }
                    default:
                    {
                        LOG(MaterialLog, Error, "Array parameter has unhandled / invalid type [{}]!", parameterName);
                        break;
                    }
                }

                break;
            }
            default:
            {
                LOG(MaterialLog, Error, "Parameter has unhandled / invalid type [{}]!", parameterName);
                break;
            }
        }
    }
}

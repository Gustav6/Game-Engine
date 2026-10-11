#include "GameFramework.pch.h"
#include "MaterialAsset.h"

#include "GraphicsEngine/GraphicsEngine.hpp"
#include "GraphicsEngine/Materials/Material.h"

#include "AssetPipeline/AssetRegistry/AssetRegistry.h"
#include "TextureAsset.h"

#include "Json/JsonWrapper.h"

#include "tracy/core/Tracy.hpp"

#include "Logger/Logger.h"

DECLARE_LOG_CATEGORY_WITH_NAME(MaterialLog, MATERIAL, Verbose);

DEFINE_LOG_CATEGORY(MaterialLog);

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
        LOG(MaterialLog, Error, "Failed to read material JSON [{}]", aPath.string());
        return false;
    }

    bool hasErrord = false;

    std::string materialName = materialData.GetString("Name");

    if (materialName.empty())
    {
        LOG(MaterialLog, Warning, "Loaded material has no name, using placeholder");
        materialName = "NO NAME GIVEN";
        hasErrord = true;
    }

    if (materialData.HasKey("Parent"))
    {
        LOG_GROUP(LogAssetRegistry, Verbose, "Instance Material");

        std::string parent = materialData.GetString("Parent");

        std::shared_ptr<MaterialAsset> masterMaterialAsset = AssetRegistry::Get().GetAsset<MaterialAsset>(parent);

        if (masterMaterialAsset == nullptr)
        {
            LOG(MaterialLog, Error, "Material [{}] failed to get master material [{}]!", materialName, parent);
            return false;
        }

        std::shared_ptr<MaterialInstance> instance = MaterialInstance::Create(materialName, masterMaterialAsset->GetMaterial());
        
	    {
            LOG_GROUP(MaterialLog, Verbose, "Instance Parameters");

		    if (materialData.HasKey("Parameters"))
		    {
		    	std::vector<JsonData> parameters = materialData.GetObjectValueArray("Parameters");

		    	if (parameters.empty())
		    	{
		    		LOG(MaterialLog, Error, "Material has no Parameters [{}]!", materialName);
                    hasErrord = true;
		    	}

		    	PushParameters(parameters, instance.get());
		    }
		    else
		    {
		    	LOG(MaterialLog, Error, "Material has no Parameters [{}]!", materialName);
                hasErrord = true;
		    }
	    }

        myMaterial = std::move(instance);
    }
    else
    {
        LOG_GROUP(MaterialLog, Verbose, "Master Material");

        MaterialDescription desc;
        desc.Name = materialName;

	    {
            LOG_GROUP(MaterialLog, Verbose, "Master Settings");

		    if (materialData.HasKey("Domain"))
		    {
                desc.Domain = static_cast<MaterialDomain>(materialData.GetInt("Domain") + 1);
		    }
		    else
		    {
                LOG(MaterialLog, Error, "Material has no Domain [{}]!", materialName);
                hasErrord = true;
		    }

        	if (materialData.HasKey("ShadingModel"))
        	{
                desc.ShadingModel = static_cast<ShadingModel>(materialData.GetInt("ShadingModel") + 1);
        	}
        	else
        	{
                LOG(MaterialLog, Error, "Material has no Shading Model [{}]!", materialName);
                hasErrord = true;
        	}

        	if (materialData.HasKey("BlendMode"))
        	{
                desc.BlendMode = static_cast<MaterialBlendMode>(materialData.GetInt("BlendMode") + 1);
        	}
        	else
        	{
                LOG(MaterialLog, Error, "Material has no Blend Mode [{}]!", materialName);
                hasErrord = true;
        	}
	    }

        if (!hasErrord)
        {

            desc.MaterialShaderCode = AssetRegistry::Get().GetWorkingDirectory() / "Shaders/BaseMaterial.hlsli";

            std::shared_ptr<Material> material = std::make_shared<Material>();

            if (!GraphicsEngine::Get().CreateMaterial(desc, *material))
            {
                LOG(MaterialLog, Error, "Failed to create material [{}]!", materialName);
                hasErrord = true;
            }

            {
                LOG_GROUP(MaterialLog, Verbose, "Master Parameters");

                if (materialData.HasKey("Parameters"))
                {
                    std::vector<JsonData> parameters = materialData.GetObjectValueArray("Parameters");

                    if (parameters.empty())
                    {
                        LOG(MaterialLog, Error, "Material has no Parameters [{}]!", materialName);
                        hasErrord = true;
                    }

                    PushParameters(parameters, material.get());
                }
                else
                {
                    LOG(MaterialLog, Error, "Material has no Parameters [{}]!", materialName);
                    hasErrord = true;
                }
            }

            myMaterial = std::move(material);
        }
    }

    return !hasErrord;
}

bool MaterialAsset::Construct(const JsonData& aMaterialData)
{
    if (!aMaterialData.IsValid())
    {
        LOG(MaterialLog, Error, "Cannot construct material from invalid JSON data!");
        return false;
    }

    bool hasErrord = false;

    std::string materialName;

    if (aMaterialData.HasKey("Name"))
    {
	    materialName = aMaterialData.GetString("Name");
    }
    else
    {
        LOG(MaterialLog, Warning, "Constructed material has no name, using placeholder");
        materialName = "NO NAME GIVEN";
        hasErrord = true;
    }

    if (aMaterialData.HasKey("Parent"))
    {
        std::string parent = aMaterialData.GetString("Parent");

        if (parent == "Self")
        {
            parent = aMaterialData.GetString("Name");
        }

        std::shared_ptr<MaterialAsset> masterMaterialAsset = AssetRegistry::Get().GetAsset<MaterialAsset>(parent);

        if (masterMaterialAsset == nullptr)
        {
            LOG(MaterialLog, Error, "Material [{}] failed to get master material [{}]!", materialName, parent);
        	return false;
        }

        std::shared_ptr<MaterialInstance> instance = MaterialInstance::Create(materialName, masterMaterialAsset->GetMaterial());

	    {
            LOG_GROUP(MaterialLog, Verbose, "Instance Parameters");

		    if (aMaterialData.HasKey("Parameters"))
		    {
		    	std::vector<JsonData> parameters = aMaterialData.GetObjectValueArray("Parameters");

		    	if (parameters.empty())
		    	{
		    		LOG(MaterialLog, Error, "Material has no Parameters [{}]!", materialName);
                    hasErrord = true;
		    	}

		    	PushParameters(parameters, instance.get());
		    }
		    else
		    {
		    	LOG(MaterialLog, Error, "Material has no Parameters [{}]!", materialName);
		    	hasErrord = true;
		    }
	    }

        myMaterial = std::move(instance);
    }
    else
    {
        MaterialDescription desc;
        desc.Name = materialName;

        if (aMaterialData.HasKey("Domain"))
        {
            desc.Domain = static_cast<MaterialDomain>(aMaterialData.GetInt("Domain") + 1);
        }
        else
        {
            LOG(MaterialLog, Error, "Material has no Domain [{}]!", materialName);
            hasErrord = true;
        }

        if (aMaterialData.HasKey("ShadingModel"))
        {
            desc.ShadingModel = static_cast<ShadingModel>(aMaterialData.GetInt("ShadingModel") + 1);
        }
        else
        {
            LOG(MaterialLog, Error, "Material has no Shading Model [{}]!", materialName);
            hasErrord = true;
        }

        if (aMaterialData.HasKey("BlendMode"))
        {
            desc.BlendMode = static_cast<MaterialBlendMode>(aMaterialData.GetInt("BlendMode") + 1);
        }
        else
        {
            LOG(MaterialLog, Error, "Material has no Blend Mode [{}]!", materialName);
            hasErrord = true;
        }

        if (!hasErrord)
        {
            desc.MaterialShaderCode = AssetRegistry::Get().GetWorkingDirectory() / "Shaders/BaseMaterial.hlsli";

            std::shared_ptr<Material> material = std::make_shared<Material>();

            if (!GraphicsEngine::Get().CreateMaterial(desc, *material))
            {
                LOG(MaterialLog, Error, "Failed to create material [{}]!", materialName);
                hasErrord = true;
            }

	        {
                LOG_GROUP(MaterialLog, Verbose, "Master Parameters");

		        if (aMaterialData.HasKey("Parameters"))
		        {
		        	std::vector<JsonData> parameters = aMaterialData.GetObjectValueArray("Parameters");

		        	if (parameters.empty())
		        	{
		        		LOG(MaterialLog, Error, "Material has no Parameters [{}]!", materialName);
		        		hasErrord = true;
		        	}

		        	PushParameters(parameters, material.get());
		        }
		        else
		        {
		        	LOG(MaterialLog, Error, "Material has no Parameters [{}]!", materialName);
		        	hasErrord = true;
		        }
	        }

            myMaterial = std::move(material);
        }
    }

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
                    LOG(MaterialLog, Warning, "Texture parameter [{}] has no texture Name, skipping", parameterName);
                    break;
                }

                std::string textureName = parameterData.GetString("Name");

                std::shared_ptr<TextureAsset> textureAsset = AssetRegistry::Get().GetAsset<TextureAsset>(textureName);

                if (textureAsset == nullptr)
                {
                    LOG(MaterialLog, Warning, "Texture parameter [{}] could not load texture [{}]", parameterName, textureName);
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
                    LOG(MaterialLog, Warning, "Texture [{}] has unknown suffix [{}]!", textureName, suffix);
                }

				break;
			}
			default:
			{
                LOG(MaterialLog, Error, "Non handled parameter type found [{}], Parameter [{}]!", type, aMaterialInstance->GetName());
				break;
			}
		}
	}
}
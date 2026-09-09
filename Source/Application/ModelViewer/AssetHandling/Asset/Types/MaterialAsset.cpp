#include "MaterialAsset.h"

#include "GraphicsEngine/GraphicsEngine.hpp"

#include "Json/JsonWrapper.h"

#include <tracy/core/Tracy.hpp>

MaterialAsset::~MaterialAsset() = default;

const Material& MaterialAsset::GetMaterial() const
{
    return myMaterial;
}

bool MaterialAsset::Load(const std::filesystem::path& aPath)
{
    /*ZoneScopedN("Load Material");

    JsonData materialData = Json::Read(aPath);

    if (!materialData.IsValid())
    {
        return false;
    }

    if (materialData.HasKey("ParentMaterial"))
    {
        std::string_view parent = materialData.GetString("ParentMaterial");
        std::filesystem::path parentPath = std::filesystem::path(parent).lexically_normal();

        auto it = myMaterials.find(parentPath);

        if (it == myMaterials.end())
        {
            if (!LoadMaterial(parentPath))
            {
                return false;
            }

            it = myMaterials.find(parentPath);
        }

        auto instance = MaterialInstance::Create(materialData.GetString("Name"), it->second);

        if (materialData.HasKey("Textures"))
        {
            JsonData textureSettings = materialData.GetObjectValue("Textures");

            std::filesystem::path albedoTexturePath = textureSettings.GetString("AlbedoTexture");
            std::filesystem::path normalTexturePath = textureSettings.GetString("NormalTexture");
            std::filesystem::path materialTexturePath = textureSettings.GetString("MaterialTexture");

            if (!LoadTexture(albedoTexturePath))
            {
                return false;
            }
            if (!LoadTexture(normalTexturePath))
            {
                return false;
            }
            if (!LoadTexture(materialTexturePath))
            {
                return false;
            }

            instance->SetTexture(0, GetTexture(albedoTexturePath.string()));
            instance->SetTexture(1, GetTexture(normalTexturePath.string()));
            instance->SetTexture(2, GetTexture(materialTexturePath.string()));

            if (materialData.HasKey("Shader"))
            {
                JsonData shaderSettings = materialData.GetObjectValue("Shader");

                if (shaderSettings.HasKey("Parameters"))
                {
                    JsonData materialParameters = shaderSettings.GetObjectValue("Parameters");

                    if (materialParameters.HasKey("MB_Tint"))
                    {
                        std::vector<float> value = materialParameters.GetFloatArray("MB_Tint");

                        if (!value.empty())
                        {
                            CommonUtilities::Vector4f tint =
                            {
                                value[0],
                                value[1],
                                value[2],
                                value[3]
                            };

                            instance->SetValue("MB_Tint", tint);
                        }
                    }

                    if (materialParameters.HasKey("MB_Alpha"))
                    {
                        instance->SetValue("MB_Alpha", materialParameters.GetFloat("MB_Alpha"));
                    }
                }
            }
        }
        else
        {
            LOG(MaterialLog, Error, "Could not find material's texture settings in json [{}]", normalizedPath.string());
        }

        myMaterials.emplace(normalizedPath, std::move(instance));

        LOG(MaterialLog, Verbose, "Material instance has loaded [{}]", normalizedPath.string());

        return true;
    }

    MaterialDescription desc;
    desc.Name = materialData.GetString("Name");

    if (materialData.HasKey("Settings"))
    {
        JsonData materialSettings = materialData.GetObjectValue("Settings");

        desc.Domain = static_cast<MaterialDomain>(materialSettings.GetInt("MaterialDomain"));
        desc.ShadingModel = static_cast<ShadingModel>(materialSettings.GetInt("ShadingModel"));
        desc.BlendMode = static_cast<MaterialBlendMode>(materialSettings.GetInt("BlendMode"));
    }
    else
    {
        LOG(MaterialLog, Error, "Could not load material's base settings in json [{}]", normalizedPath.string());
    }

    std::filesystem::path albedoTexturePath;
    std::filesystem::path normalTexturePath;
    std::filesystem::path materialTexturePath;

    if (materialData.HasKey("Textures"))
    {
        JsonData textureSettings = materialData.GetObjectValue("Textures");

        albedoTexturePath = textureSettings.GetString("AlbedoTexture");
        normalTexturePath = textureSettings.GetString("NormalTexture");
        materialTexturePath = textureSettings.GetString("MaterialTexture");

        if (!LoadTexture(albedoTexturePath))
        {
            return false;
        }
        if (!LoadTexture(normalTexturePath))
        {
            return false;
        }
        if (!LoadTexture(materialTexturePath))
        {
            return false;
        }
    }
    else
    {
        LOG(MaterialLog, Error, "Could not find material's texture settings in json [{}]", normalizedPath.string());
    }

    if (materialData.HasKey("Shader"))
    {
        JsonData shaderSettings = materialData.GetObjectValue("Shader");

        if (shaderSettings.HasKey("Code"))
        {
            desc.MaterialShaderCode = myContentRoot / shaderSettings.GetString("Code");
        }
    }

    if (!GraphicsEngine::Get().CreateMaterial(desc, myMaterial))
    {
        return false;
    }

    myMaterial.SetTexture(0, GetTexture(albedoTexturePath.string()));
    myMaterial.SetTexture(1, GetTexture(normalTexturePath.string()));
    myMaterial.SetTexture(2, GetTexture(materialTexturePath.string()));

    LOG(MaterialLog, Verbose, "Material has loaded [{}]", aPath.string());*/

    return true;
}

#include "MaterialAsset.h"

#include "Application.h"

#include "GraphicsEngine/GraphicsEngine.hpp"

#include "TextureAsset.h"

#include "Json/JsonWrapper.h"

#include <tracy/core/Tracy.hpp>

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

    if (materialData.HasKey("ParentMaterial"))
    {
        std::shared_ptr<MaterialAsset> parentMaterial = Application.AssetRegistry.GetAsset<MaterialAsset>(materialData.GetString("ParentMaterial"));

        auto instance = MaterialInstance::Create(materialData.GetString("Name"), parentMaterial->GetMaterial());

        if (materialData.HasKey("Textures"))
        {
            JsonData textureSettings = materialData.GetObjectValue("Textures");

            myTextureAssets.push_back(Application.AssetRegistry.GetAsset<TextureAsset>(textureSettings.GetString("AlbedoTexture")));
            myTextureAssets.push_back(Application.AssetRegistry.GetAsset<TextureAsset>(textureSettings.GetString("NormalTexture")));
            myTextureAssets.push_back(Application.AssetRegistry.GetAsset<TextureAsset>(textureSettings.GetString("MaterialTexture")));

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
            LOG(MaterialLog, Error, "Could not find material's texture settings in json [{}]", aPath.string());
        }

        instance->SetTexture(0, myTextureAssets[0]->GetTexture());
        instance->SetTexture(1, myTextureAssets[1]->GetTexture());
        instance->SetTexture(2, myTextureAssets[2]->GetTexture());

        myMaterial = std::move(instance);

        LOG(MaterialLog, Verbose, "Material instance has loaded [{}]", aPath.string());

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
        LOG(MaterialLog, Error, "Could not load material's base settings in json [{}]", aPath.string());
    }

    std::filesystem::path albedoTexturePath;
    std::filesystem::path normalTexturePath;
    std::filesystem::path materialTexturePath;

    if (materialData.HasKey("Textures"))
    {
        JsonData textureSettings = materialData.GetObjectValue("Textures");

        myTextureAssets.push_back(Application.AssetRegistry.GetAsset<TextureAsset>(textureSettings.GetString("AlbedoTexture")));
        myTextureAssets.push_back(Application.AssetRegistry.GetAsset<TextureAsset>(textureSettings.GetString("NormalTexture")));
        myTextureAssets.push_back(Application.AssetRegistry.GetAsset<TextureAsset>(textureSettings.GetString("MaterialTexture")));
    }
    else
    {
        LOG(MaterialLog, Error, "Could not find material's texture settings in json [{}]", aPath.string());
    }

    if (materialData.HasKey("Shader"))
    {
        JsonData shaderSettings = materialData.GetObjectValue("Shader");

        if (shaderSettings.HasKey("Code"))
        {
            desc.MaterialShaderCode = Application.AssetRegistry.GetWorkingDirectory() / shaderSettings.GetString("Code");
        }
    }

    std::shared_ptr<Material> material = std::make_shared<Material>();

    if (!GraphicsEngine::Get().CreateMaterial(desc, *material))
    {
        return false;
    }

    material->SetTexture(0, myTextureAssets[0]->GetTexture());
    material->SetTexture(1, myTextureAssets[1]->GetTexture());
    material->SetTexture(2, myTextureAssets[2]->GetTexture());

    myMaterial = std::move(material);

    LOG(MaterialLog, Verbose, "Material has loaded [{}]", aPath.string());

    return true;
}

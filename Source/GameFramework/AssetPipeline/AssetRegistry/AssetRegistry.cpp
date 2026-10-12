#include "GameFramework.pch.h"
#include "AssetRegistry.h"

#include "AssetPipeline/Assets/Types/MeshAsset.h"

#include "CommonUtilities/Collision/Sphere.hpp"
#include "CommonUtilities/Collision/AABB3D.hpp"
#include "GraphicsEngine/Objects/GPU/Vertex.h"
#include "GraphicsEngine/Objects/Mesh/Mesh.h"
#include "GraphicsEngine/Objects/Mesh/Skeleton.h"

#include "CommonUtilities/Math/MathUtils.hpp"

#include "ensure.h"
#include "tgafbx/Importer.h"

DEFINE_LOG_CATEGORY(LogAssetRegistry);

AssetRegistry& AssetRegistry::Get()
{
    static AssetRegistry myInstance;
    return myInstance;
}

AssetRegistry::AssetRegistry() = default;
AssetRegistry::~AssetRegistry() = default;

bool AssetRegistry::Initialize(std::filesystem::path aWorkingDirectory)
{
	ensure(myAssets.empty());

    myWorkingDirectory = std::filesystem::absolute(aWorkingDirectory);

    TGA::FBX::Importer::InitImporter();

    InitUnitAxes();
    InitUnitCube();
    InitUnitSphere();
	InitUnitPlane();

    if (!std::filesystem::exists(myWorkingDirectory))
    {
        LOG(LogAssetRegistry, Fatal, "Content root dir is not a valid path! Provided root was {}.", aWorkingDirectory.string());
        return false;
    }

    for (const auto& item : std::filesystem::recursive_directory_iterator(myWorkingDirectory))
    {
        if (item.is_regular_file() && item.path().has_extension())
        {
            std::string lowerCaseFileExtension = item.path().extension().string();
            std::ranges::transform(lowerCaseFileExtension, lowerCaseFileExtension.begin(), tolower);

            if (myFileExtensionToAssetTypeMap.contains(lowerCaseFileExtension))
            {
                std::string assetName = item.path().stem().string();
                std::ranges::transform(assetName, assetName.begin(), tolower);

                AssetInformation asset;

                asset.Path = item.path();
                myAssets.emplace(assetName, asset);
            }
        }
    }

    return true;
}

void AssetRegistry::UnInitialize()
{
    TGA::FBX::Importer::UninitImporter();
    myAssets.clear();
}

const std::filesystem::path& AssetRegistry::GetWorkingDirectory() const
{
    return myWorkingDirectory;
}

void AssetRegistry::RegisterAssetTypeInternal(const std::vector<std::string>& aExtensionList, std::type_index aTypeIndex)
{
    for (const auto& extension : aExtensionList)
    {
        std::string lowerCaseFileExtension = extension;
        std::ranges::transform(lowerCaseFileExtension, lowerCaseFileExtension.begin(), tolower);

        myFileExtensionToAssetTypeMap[lowerCaseFileExtension].insert(aTypeIndex);
    }
}
std::string_view AssetRegistry::RemoveTypePrefix(std::string_view aTypeName)
{
    for (std::string_view prefix : { "class ", "struct " })
    {
        if (aTypeName.starts_with(prefix))
        {
            return aTypeName.substr(prefix.size());
        }
    }

    return aTypeName;
}

void AssetRegistry::InitUnitCube()
{
    std::vector<Vertex> meshVertices =
    {
        // Front
        {.Position = {-0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 0, 0,-1 }, .Tangent = { 1, 0, 0 } },
        {.Position = {-0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 0, 0,-1 }, .Tangent = { 1, 0, 0 } },
        {.Position = { 0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 0, 0,-1 }, .Tangent = { 1, 0, 0 } },
        {.Position = { 0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 0, 0,-1 }, .Tangent = { 1, 0, 0 } },

        // Back
        {.Position = {-0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 0, 0, 1 }, .Tangent = {-1, 0, 0 } },
        {.Position = {-0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 0, 0, 1 }, .Tangent = {-1, 0, 0 } },
        {.Position = { 0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 0, 0, 1 }, .Tangent = {-1, 0, 0 } },
        {.Position = { 0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 0, 0, 1 }, .Tangent = {-1, 0, 0 } },

        // Left
        {.Position = {-0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = {-1, 0, 0 }, .Tangent = { 0, 0,-1 } },
        {.Position = {-0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = {-1, 0, 0 }, .Tangent = { 0, 0,-1 } },
        {.Position = {-0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = {-1, 0, 0 }, .Tangent = { 0, 0,-1 } },
        {.Position = {-0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = {-1, 0, 0 }, .Tangent = { 0, 0,-1 } },

        // Right
        {.Position = { 0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 1, 0, 0 }, .Tangent = { 0, 0, 1 } },
        {.Position = { 0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 1, 0, 0 }, .Tangent = { 0, 0, 1 } },
        {.Position = { 0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 1, 0, 0 }, .Tangent = { 0, 0, 1 } },
        {.Position = { 0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 1, 0, 0 }, .Tangent = { 0, 0, 1 } },

        // Top
        {.Position = {-0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 0, 1, 0 }, .Tangent = {-1, 0, 0 } },
        {.Position = {-0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 0, 1, 0 }, .Tangent = {-1, 0, 0 } },
        {.Position = { 0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 0, 1, 0 }, .Tangent = {-1, 0, 0 } },
        {.Position = { 0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 0, 1, 0 }, .Tangent = {-1, 0, 0 } },

        // Bottom
        {.Position = {-0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 0,-1, 0 }, .Tangent = { 1, 0, 0 } },
        {.Position = {-0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 0,-1, 0 }, .Tangent = { 1, 0, 0 } },
        {.Position = { 0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 0,-1, 0 }, .Tangent = { 1, 0, 0 } },
        {.Position = { 0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 0,-1, 0 }, .Tangent = { 1, 0, 0 } },
    };

    std::vector<unsigned> meshIndices =
    {
        // Front
        0, 1, 3,
        3, 2, 0,

        // Back
        4, 7, 5,
        7, 4, 6,

        // Left
        8, 11, 9,
        11, 8, 10,

        // Right
        12, 13, 15,
        15, 14, 12,

        // Top
        16, 17, 19,
        19, 18, 16,

        // Bottom
        20, 23, 21,
        23, 20, 22
    };

    CommonUtilities::Sphere<float> boundingSphere;
    boundingSphere.InitWithCenterAndRadius(CommonUtilities::Vector3f::Zero, 0.75f);

    CommonUtilities::AABB3D<float> boundingBox;
    boundingBox.InitWithMinAndMax({ -0.5f, -0.5f , -0.5f }, { 0.5f, 0.5f , 0.5f });

    Mesh::Element element;

    element.NumVertices = static_cast<unsigned>(meshVertices.size());
    element.NumIndices = static_cast<unsigned>(meshIndices.size());
    element.BoundingBox = boundingBox;
    element.BoundingSphere = boundingSphere;

    std::shared_ptr<MeshAsset> asset = std::make_shared<MeshAsset>();

    std::string assetName = "unit_cube";

    asset->myMesh->Initialize(assetName, {element }, std::move(meshVertices), std::move(meshIndices), nullptr, boundingSphere, boundingBox);

    AssetInformation assetInformation;

    assetInformation.Path = "Internal";
    assetInformation.Asset = asset;

    myAssets.emplace(assetName, assetInformation);

    myPersistentAssets.emplace_back(asset);
}

void AssetRegistry::InitUnitSphere()
{
    std::vector<Vertex> meshVertices;
    std::vector<unsigned> meshIndices;

    constexpr unsigned int sectorCount = 36;
    constexpr unsigned int stackCount = 18;

    constexpr float sectorStep = 2.0f * CommonUtilities::Math::PI<float> / sectorCount;
    constexpr float stackStep = CommonUtilities::Math::PI<float> / stackCount;

    Vertex vertex;

    for (unsigned int i = 0; i <= stackCount; ++i)
    {
        float stackAngle = CommonUtilities::Math::PI<float> / 2.0f - static_cast<float>(i) * stackStep;
        float xy = std::cos(stackAngle);
        float z = std::sin(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j)
        {
            float sectorAngle = static_cast<float>(j) * sectorStep;

            vertex.Position.x = xy * std::cos(sectorAngle) * 0.5f;
            vertex.Position.y = xy * std::sin(sectorAngle) * 0.5f;
            vertex.Position.z = z * 0.5f;
            vertex.Position.w = 1.0f;

            vertex.Color = { 1.0f, 1.0f, 1.0f, 1.0f };

            vertex.UV0.x = static_cast<float>(j) / sectorCount;
            vertex.UV0.y = static_cast<float>(i) / stackCount;

            vertex.Normal.x = xy * std::cos(sectorAngle);
            vertex.Normal.y = xy * std::sin(sectorAngle);
            vertex.Normal.z = z;

            vertex.Tangent.x = -std::sin(sectorAngle);
            vertex.Tangent.y = std::cos(sectorAngle);
            vertex.Tangent.z = 0.0f;

            meshVertices.emplace_back(vertex);
        }
    }

    for (unsigned int i = 0; i < stackCount; ++i)
    {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                meshIndices.push_back(k1);
                meshIndices.push_back(k2);
                meshIndices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1))
            {
                meshIndices.push_back(k1 + 1);
                meshIndices.push_back(k2);
                meshIndices.push_back(k2 + 1);
            }
        }
    }

    CommonUtilities::Sphere<float> boundingSphere;
    boundingSphere.InitWithCenterAndRadius(CommonUtilities::Vector3f::Zero, 0.5f);

    CommonUtilities::AABB3D<float> boundingBox;
    boundingBox.InitWithMinAndMax({ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f });

    Mesh::Element element;
    element.NumVertices = static_cast<unsigned>(meshVertices.size());
    element.NumIndices = static_cast<unsigned>(meshIndices.size());
    element.BoundingBox = boundingBox;
    element.BoundingSphere = boundingSphere;

    std::shared_ptr<MeshAsset> asset = std::make_shared<MeshAsset>();
    std::string assetName = "unit_sphere";

    asset->myMesh->Initialize(assetName, { element }, std::move(meshVertices), std::move(meshIndices), nullptr, boundingSphere, boundingBox);

    AssetInformation assetInformation;
    assetInformation.Path = "Internal";
    assetInformation.Asset = asset;

    myAssets.emplace(assetName, assetInformation);
    myPersistentAssets.emplace_back(asset);
}

void AssetRegistry::InitUnitPlane()
{
	std::vector<Vertex> meshVertices =
	{
		{.Position = {-0.5f, 0, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 0, 1, 0 }, .Tangent = {-1, 0, 0 } },
		{.Position = {-0.5f, 0,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 0, 1, 0 }, .Tangent = {-1, 0, 0 } },
		{.Position = { 0.5f, 0, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 0, 1, 0 }, .Tangent = {-1, 0, 0 } },
		{.Position = { 0.5f, 0,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 0, 1, 0 }, .Tangent = {-1, 0, 0 } }
	};

	std::vector<unsigned> meshIndices =
	{
		0, 1, 3,
		3, 2, 0,
	};

    CommonUtilities::Sphere<float> boundingSphere;
    boundingSphere.InitWithCenterAndRadius(CommonUtilities::Vector3f::Zero, 0.75f);

    CommonUtilities::AABB3D<float> boundingBox;
    boundingBox.InitWithMinAndMax({ -0.5f, 0.0f , -0.5f }, { 0.5f, 0.0f , 0.5f });

	Mesh::Element element;

	element.NumVertices = static_cast<unsigned>(meshVertices.size());
	element.NumIndices = static_cast<unsigned>(meshIndices.size());
    element.BoundingBox = boundingBox;
    element.BoundingSphere = boundingSphere;

    std::shared_ptr<MeshAsset> asset = std::make_shared<MeshAsset>();

    std::string assetName = "unit_plane";

    asset->myMesh->Initialize(assetName, { element }, std::move(meshVertices), std::move(meshIndices), nullptr, boundingSphere, boundingBox);

    AssetInformation assetInformation;

    assetInformation.Path = "Internal";
    assetInformation.Asset = asset;

    myAssets.emplace(assetName, assetInformation);

    myPersistentAssets.emplace_back(asset);
}

void AssetRegistry::InitUnitAxes()
{
}
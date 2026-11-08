#include "Core/Core.pch.h"
#include "AssetRegistry.h"

#include "Core/EngineContext.h"
#include "Core/Assets/Concrete/MeshAsset.h"

#include "Graphics/Objects/GPU/Vertex.h"
#include "Graphics/Objects/Mesh/Mesh.h"
#include "Graphics/Objects/Mesh/Skeleton.h"

AssetRegistry& AssetRegistry::Get()
{
    static AssetRegistry myInstance;
    return myInstance;
}

AssetRegistry::AssetRegistry() = default;
AssetRegistry::~AssetRegistry() = default;

bool AssetRegistry::Initialize()
{
	ensure(myAssets.empty());

    TGA::FBX::Importer::InitImporter();

    InitUnitAxes();
    InitUnitCube();
    InitUnitSphere();
	InitUnitPlane();

    if (!std::filesystem::exists(EngineContext.ContentRoot))
    {
        SPDLOG_CRITICAL("Content root dir is not a valid path! Root is [{}].", EngineContext.ContentRoot.string());
        return false;
    }

    for (const auto& item : std::filesystem::recursive_directory_iterator(EngineContext.ContentRoot))
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

void AssetRegistry::Shutdown()
{
    TGA::FBX::Importer::UninitImporter();
    myAssets.clear();
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
	boundingSphere.InitWithCenterAndRadius(CommonUtilities::Vector3f::Zero, 0.8661f); // ~ sqrt(3) / 2

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
    // Resource:
	// https://gist.github.com/Pikachuxxxx/5c4c490a7d7679824e0e18af42918efc

    constexpr float radius = 0.5f; 
	
	constexpr int latitudes = 18; 
	constexpr int longitudes = 36;

    constexpr float deltaLatitude = CommonUtilities::Math::PI<float> / latitudes;
    constexpr float deltaLongitude = 2 * CommonUtilities::Math::PI<float> / longitudes;

    std::vector<Vertex> meshVertices;
    std::vector<unsigned int> meshIndices;

    for (int i = 0; i <= latitudes; ++i)
    {
        float latitudeAngle = CommonUtilities::Math::PI<float> / 2 - static_cast<float>(i) * deltaLatitude;
        float xy = radius * cosf(latitudeAngle);
        float z = radius * sinf(latitudeAngle);

        for (int j = 0; j <= longitudes; ++j)
        {
            float longitudeAngle = static_cast<float>(j) * deltaLongitude;

            Vertex vertex;
            vertex.Position.x = xy * cosf(longitudeAngle);
            vertex.Position.y = xy * sinf(longitudeAngle);
            vertex.Position.z = z;
            vertex.Position.w = 1.0f;

            vertex.Color = { 1.0f, 1.0f, 1.0f, 1.0f };

            vertex.UV0.x = static_cast<float>(j) / longitudes;
            vertex.UV0.y = static_cast<float>(i) / latitudes;

            vertex.Normal = vertex.Position.ToXYZ() / radius;

            vertex.Tangent.x = -sinf(longitudeAngle);
            vertex.Tangent.y = cosf(longitudeAngle);
            vertex.Tangent.z = 0.0f;

			meshVertices.emplace_back(vertex);
        }
    }

    unsigned int k1, k2;

    for (unsigned int i = 0; i < latitudes; ++i)
    {
        k1 = i * (longitudes + 1);
        k2 = k1 + longitudes + 1;

        for (int j = 0; j < longitudes; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                meshIndices.push_back(k1);
                meshIndices.push_back(k2);
                meshIndices.push_back(k1 + 1);
            }

            if (i != (latitudes - 1))
            {
                meshIndices.push_back(k1 + 1);
                meshIndices.push_back(k2);
                meshIndices.push_back(k2 + 1);
            }
        }
    }

    CommonUtilities::Sphere<float> boundingSphere;
    boundingSphere.InitWithCenterAndRadius(CommonUtilities::Vector3f::Zero, radius);

    CommonUtilities::AABB3D<float> boundingBox;
    boundingBox.InitWithMinAndMax({ -radius, -radius, -radius }, { radius, radius, radius });

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
	boundingSphere.InitWithCenterAndRadius(CommonUtilities::Vector3f::Zero, 0.7072f); // ~ sqrt(2) / 2

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
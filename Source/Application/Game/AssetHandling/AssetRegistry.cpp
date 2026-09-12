#include "AssetRegistry.h"

#include <cctype>

#include "GraphicsEngine/Objects/Vertex.h"
#include "GraphicsEngine/Objects/Mesh.h"

#include "tgafbx/Importer.h"

#include "Asset/Types/MeshAsset.h"

AssetRegistry::AssetRegistry() = default;
AssetRegistry::~AssetRegistry() = default;

void AssetRegistry::Initialize(std::filesystem::path aWorkingDirectory)
{
	ensure(myAssets.empty());

    myWorkingDirectory = std::filesystem::absolute(aWorkingDirectory);

    TGA::FBX::Importer::InitImporter();

    InitUnitAxes();
    InitUnitCube();
	InitUnitPlane();

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
}

void AssetRegistry::UnInitialize()
{
    TGA::FBX::Importer::UninitImporter();
    myAssets.clear();
}

const std::filesystem::path& AssetRegistry::GetWorkingDirectory()
{
    return myWorkingDirectory;
}

void AssetRegistry::RegisterAssetTypeInternal(const std::vector<std::string>& aExtensionList, const MakeAssetFunction& aMakeAssetFunction)
{
	for (const auto& extension : aExtensionList)
	{
		std::string lowerCaseFileExtension = extension;
		std::ranges::transform(lowerCaseFileExtension, lowerCaseFileExtension.begin(), tolower);

		if (!myFileExtensionToAssetTypeMap.contains(lowerCaseFileExtension))
		{
			std::vector<MakeAssetFunction> functions;
			myFileExtensionToAssetTypeMap.emplace(lowerCaseFileExtension, functions);
		}

		myFileExtensionToAssetTypeMap.at(lowerCaseFileExtension).emplace_back(aMakeAssetFunction);
	}
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

    Mesh::Element element;

    element.NumVertices = static_cast<unsigned>(meshVertices.size());
    element.NumIndices = static_cast<unsigned>(meshIndices.size());

    // TODO: Calculate bounding box / sphere for unit cube
    //std::unique_ptr<CommonUtilities::Sphere<float>> boundingSphere = nullptr;

    std::shared_ptr<MeshAsset> asset = std::make_shared<MeshAsset>();;

    std::string assetName = "unit_cube";

    asset->myMesh.Initialize(assetName, {element}, std::move(meshVertices), std::move(meshIndices), nullptr, nullptr);

    AssetInformation assetInformation;

    assetInformation.Path = "Internal";
    assetInformation.Asset = asset;

    myAssets.emplace(assetName, assetInformation);

    myPersistentAssets.emplace(assetName, asset);
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

	Mesh::Element element;

	element.NumVertices = static_cast<unsigned>(meshVertices.size());
	element.NumIndices = static_cast<unsigned>(meshIndices.size());

	// TODO: Calculate bounding box / sphere for unit plane
	//std::unique_ptr<CommonUtilities::Sphere<float>> boundingSphere = nullptr;

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    std::shared_ptr<MeshAsset> asset = std::make_shared<MeshAsset>();;

    std::string assetName = "unit_plane";

    asset->myMesh.Initialize(assetName, { element }, std::move(meshVertices), std::move(meshIndices), nullptr, nullptr);

    AssetInformation assetInformation;

    assetInformation.Path = "Internal";
    assetInformation.Asset = asset;

    myAssets.emplace(assetName, assetInformation);

    myPersistentAssets.emplace(assetName, asset);
}

void AssetRegistry::InitUnitAxes()
{
}
#include "AssetLibrary.h"

#include <string>
#include <filesystem>

#include "GraphicsEngine/GraphicsEngine.hpp"

#include "GraphicsEngine/Objects/Mesh.h"
#include "GraphicsEngine/Objects/Skeleton.h"
#include "GraphicsEngine/Objects/Animation.h"
#include "GraphicsEngine/Objects/Texture.h"
#include "GraphicsEngine/Objects/Vertex.h"
#include "GraphicsEngine/Materials/Material.h"

#include "tgafbx/Importer.h"
#include "tgafbx/TgaFbxStructs.h"

#include "CommonUtilities/Sphere.hpp"

#include "Logger/Logger.h"

#include "Json/JsonWrapper.h"

#include <tracy/core/Tracy.hpp>

DECLARE_LOG_CATEGORY_WITH_NAME(FBXLog, FBX, Verbose);

DEFINE_LOG_CATEGORY(FBXLog);

AssetLibrary::AssetLibrary() = default;
AssetLibrary::~AssetLibrary() = default;

void AssetLibrary::Initialize(const std::filesystem::path& aContentRoot)
{
    TGA::FBX::Importer::InitImporter();

    myContentRoot = aContentRoot;

    InitUnitCube();
    InitUnitPlane();
    InitUnitAxes();
}

void AssetLibrary::UnInitialize()
{
    TGA::FBX::Importer::UninitImporter();
}

std::shared_ptr<Mesh> AssetLibrary::GetMesh(std::string_view aName)
{
    auto it = myMeshes.find(aName.data());

    if (it == myMeshes.end())
    {
        LOG(FBXLog, Error, "Could not find Mesh [{}]", aName);
        return nullptr;
    }

    return it->second;
}

std::shared_ptr<Animation> AssetLibrary::GetAnimation(std::string_view aName)
{
    auto it = myAnimations.find(aName.data());

    if (it == myAnimations.end())
    {
        LOG(FBXLog, Error, "Could not find Animation [{}]", aName);
        return nullptr;
    }

    return it->second;
}

std::shared_ptr<Texture> AssetLibrary::GetTexture(std::string_view aName)
{
    auto it = myTextures.find(aName.data());

    if (it == myTextures.end())
    {
        LOG(FBXLog, Error, "Could not find Texture [{}]", aName);
        return nullptr;
    }

    return it->second;
}

std::shared_ptr<MaterialInterface> AssetLibrary::GetMaterial(std::string_view aName)
{
    auto it = myMaterials.find(aName.data());

    if (it == myMaterials.end())
    {
        LOG(FBXLog, Error, "Could not find Material [{}]", aName);
        return nullptr;
    }

    return it->second;
}

bool AssetLibrary::LoadMesh(const std::filesystem::path& aPath)
{
    ZoneScopedN("Load Mesh");

    const std::filesystem::path normalizedPath = aPath.lexically_normal();
    const std::filesystem::path fullPath = myContentRoot / normalizedPath;

    if (myMeshes.contains(normalizedPath))
    {
        return true;
    }

    std::unique_ptr<TGA::FBX::Mesh> tgaMesh = std::make_unique<TGA::FBX::Mesh>();
    const TGA::FBX::FbxImportStatus& importStatus = TGA::FBX::Importer::LoadMesh(fullPath, *tgaMesh);

    if (!importStatus)
    {
        const std::string pathString = normalizedPath.string();

        switch (importStatus.Result)
        {
            case TGA::FBX::FbxImportResult::InternalError:
                LOG(FBXLog, Error, "Could not load mesh [{}] : Internal FBX importer error!", pathString);
                break;
            case TGA::FBX::FbxImportResult::NotInitialized:
                LOG(FBXLog, Error, "Could not load mesh [{}] : FBX importer is not initialized!", pathString);
                break;
            case TGA::FBX::FbxImportResult::FileNotFound:
                LOG(FBXLog, Error, "Could not load mesh [{}] : File was not found!", pathString);
                break;
            case TGA::FBX::FbxImportResult::NoMeshData:
                LOG(FBXLog, Error, "Could not load mesh [{}] : No mesh data found in file!", pathString);
                break;
            default:
                LOG(FBXLog, Error, "Could not load mesh [{}] : Unknown error!", pathString);
                break;
        }

        return false;
    }

    for (const auto& [warning, amount] : importStatus.Warnings)
    {
        const std::string pathString = normalizedPath.string();

        switch (warning)
        {
            case TGA::FBX::FbxImportWarning::TooManySkinWeights:
                LOG(FBXLog, Warning, "Mesh [{}] : Vertices had too many skin weights and were truncated!", pathString);
                break;
            case TGA::FBX::FbxImportWarning::MissingSkinWeights:
                LOG(FBXLog, Warning, "Mesh [{}] : Vertices are missing skin weights!", pathString);
                break;
            case TGA::FBX::FbxImportWarning::NoUVCoordinates:
                LOG(FBXLog, Warning, "Mesh [{}] : Surfaces have no UV coordinates!", pathString);
                break;
            case TGA::FBX::FbxImportWarning::GenerateNormalsFailed:
                LOG(FBXLog, Warning, "Mesh [{}] : Failed to generate normal's for surfaces!", pathString);
                break;
            case TGA::FBX::FbxImportWarning::CannotTriangulate:
                LOG(FBXLog, Warning, "Mesh [{}] : Could not triangulate polygons!", pathString);
                break;
            case TGA::FBX::FbxImportWarning::DuplicateBoneNames:
                LOG(FBXLog, Warning, "Mesh [{}] : Duplicate bone names detected!", pathString);
                break;
            default:
                LOG(FBXLog, Warning, "Mesh [{}] : Unknown warning!", pathString);
                break;
        }
    }

    std::vector<Mesh::Element> elements;
    elements.reserve(tgaMesh->Elements.size());

    std::vector<Vertex> meshVertices;
    std::vector<unsigned> meshIndices;

    for (TGA::FBX::Mesh::Element& tgaElement : tgaMesh->Elements)
    {
        Mesh::Element element;
        element.IndexOffset = static_cast<unsigned int>(meshIndices.size());
        element.VertexOffset = static_cast<unsigned int>(meshVertices.size());
        element.NumVertices = static_cast<unsigned int>(tgaElement.Vertices.size());
        element.NumIndices = static_cast<unsigned int>(tgaElement.Indices.size());
        element.MaterialIndex = tgaElement.MaterialIndex;

        meshVertices.reserve(meshVertices.size() + tgaElement.Vertices.size());
        for (auto& tgaVertex : tgaElement.Vertices)
        {
            Vertex vertex;
            memcpy_s(&vertex.UV0, sizeof(CommonUtilities::Vector2f), &tgaVertex.UVs, sizeof(float) * 2);
            memcpy_s(&vertex.Position, sizeof(CommonUtilities::Vector4f), &tgaVertex.Position, sizeof(float) * 4);
            memcpy_s(&vertex.BoneIDs, sizeof(CommonUtilities::Vector4ui), &tgaVertex.BoneIDs, sizeof(unsigned int) * 4);
            memcpy_s(&vertex.SkinWeights, sizeof(CommonUtilities::Vector4f), &tgaVertex.BoneWeights, sizeof(float) * 4);
            memcpy_s(&vertex.Normal, sizeof(CommonUtilities::Vector3f), &tgaVertex.Normal, sizeof(float) * 3);
            memcpy_s(&vertex.Tangent, sizeof(CommonUtilities::Vector3f), &tgaVertex.Tangent, sizeof(float) * 3);

            meshVertices.emplace_back(vertex);
        }

        meshIndices.reserve(meshIndices.size() + tgaElement.Indices.size());
        for (auto& index : tgaElement.Indices)
        {
            meshIndices.emplace_back(index + element.VertexOffset);
        }

        elements.emplace_back(std::move(element));
    }

    std::unique_ptr<Skeleton> skeleton = nullptr;

    if (!tgaMesh->Skeleton.Bones.empty())
    {
        std::vector<Skeleton::Joint> joints;
        joints.reserve(tgaMesh->Skeleton.Bones.size());

        for (TGA::FBX::Skeleton::Bone& tgaBone : tgaMesh->Skeleton.Bones)
        {
            Skeleton::Joint joint;
            memcpy_s(&joint.BindPoseInverse, sizeof(CommonUtilities::Matrix4x4f), &tgaBone.BindPoseInverse.Data, sizeof(TGA::FBX::Matrix));
            joint.BindPoseInverse = joint.BindPoseInverse.GetTranspose();
            joint.Parent = tgaBone.ParentIdx;
            joint.Children = std::move(tgaBone.Children);
            joint.Name = std::move(tgaBone.Name);

            joints.emplace_back(std::move(joint));
        }

        skeleton = std::make_unique<Skeleton>();
        skeleton->Joints = std::move(joints);
        skeleton->JointNameToIndex = std::move(tgaMesh->Skeleton.BoneNameToIndex);
    }

	std::unique_ptr<CommonUtilities::Sphere<float>> boundingSphere = std::make_unique<CommonUtilities::Sphere<float>>();
	/*boundingSphere->InitWithCenterAndRadius({ tgaMesh->BoxSphereBounds.Center[0], tgaMesh->BoxSphereBounds.Center[1], tgaMesh->BoxSphereBounds.Center[2] }, tgaMesh->BoxSphereBounds.Radius);*/

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    std::string name = aPath.filename().string();

    mesh->Initialize(name, std::move(elements), std::move(meshVertices), std::move(meshIndices), std::move(skeleton), std::move(boundingSphere));

    myMeshes.emplace(normalizedPath, std::move(mesh));

    LOG(FBXLog, Verbose, "Mesh has loaded [{}]", normalizedPath.string());

    return true;
}

bool AssetLibrary::LoadAnimation(const std::filesystem::path& aPath)
{
    ZoneScopedN("Load Animation");

    const std::filesystem::path normalizedPath = aPath.lexically_normal();
    const std::filesystem::path fullPath = myContentRoot / normalizedPath;

    if (myAnimations.contains(normalizedPath))
    {
        return true;
    }

    std::unique_ptr<TGA::FBX::Animation> tgaAnimation = std::make_unique<TGA::FBX::Animation>();
    const TGA::FBX::FbxImportStatus& importStatus = TGA::FBX::Importer::LoadAnimation(fullPath, *tgaAnimation);

    if (!importStatus)
    {
        const std::string pathString = normalizedPath.string();

        switch (importStatus.Result)
        {
            case TGA::FBX::FbxImportResult::InternalError:
                LOG(FBXLog, Error, "Could not load animation [{}] : Internal FBX importer error!", pathString);
                break;
            case TGA::FBX::FbxImportResult::NotInitialized:
                LOG(FBXLog, Error, "Could not load animation [{}] : FBX importer is not initialized!", pathString);
                break;
            case TGA::FBX::FbxImportResult::FileNotFound:
                LOG(FBXLog, Error, "Could not load animation [{}] : File not found!", pathString);
                break;
            case TGA::FBX::FbxImportResult::NoSkeletonData:
                LOG(FBXLog, Error, "Could not load animation [{}] : No skeleton data found in file!", pathString);
                break;
            case TGA::FBX::FbxImportResult::NoAnimationData:
                LOG(FBXLog, Error, "Could not load animation [{}] : No animation data found in file!", pathString);
                break;
            default:
                LOG(FBXLog, Error, "Could not load animation [{}] : Unknown error!", pathString);
                break;
        }

        return false;
    }

    for (const auto& [warning, count] : importStatus.Warnings)
    {
        const std::string pathString = normalizedPath.string();

        switch (warning)
        {
            case TGA::FBX::FbxImportWarning::TooManySkinWeights:
                LOG(FBXLog, Warning, "Animation [{}] : vertices had too many skin weights and were truncated!", pathString);
                break;
            case TGA::FBX::FbxImportWarning::MissingSkinWeights:
                LOG(FBXLog, Warning, "Animation [{}] : vertices are missing skin weights!", pathString);
                break;
            case TGA::FBX::FbxImportWarning::DuplicateBoneNames:
                LOG(FBXLog, Warning, "Animation [{}] : duplicate bone names detected, skinning may be incorrect!", pathString);
                break;
            default:
                LOG(FBXLog, Warning, "Animation [{}] : Unknown warning!", pathString);
                break;
        }
    }

    std::vector<Animation::Frame> frames;
    frames.reserve(tgaAnimation->Frames.size());

    for (TGA::FBX::Animation::Frame& tgaFrame : tgaAnimation->Frames)
    {
        Animation::Frame frame;

        for (auto& [tgaJointName, tgaJointTransform] : tgaFrame.LocalTransforms)
        {
            memcpy_s(&frame.Transforms[tgaJointName], sizeof(CommonUtilities::Matrix4x4f), &tgaJointTransform, sizeof(TGA::FBX::Matrix));
        }

        frames.emplace_back(std::move(frame));
    }

    std::shared_ptr<Animation> animation = std::make_shared<Animation>();
    animation->Frames = std::move(frames);
    memcpy_s(&animation->Duration, sizeof(float), &tgaAnimation->Duration, sizeof(float));
    memcpy_s(&animation->FramesPerSecond, sizeof(float), &tgaAnimation->FramesPerSecond, sizeof(float));

    myAnimations.emplace(normalizedPath, std::move(animation));

    LOG(FBXLog, Verbose, "Animation has loaded [{}]", normalizedPath.string());

    return true;
}

bool AssetLibrary::LoadTexture(const std::filesystem::path& aPath)
{
    ZoneScopedN("Load Texture");

    const std::filesystem::path normalizedPath = aPath.lexically_normal();
    const std::filesystem::path fullPath = myContentRoot / normalizedPath;

    if (myTextures.contains(normalizedPath))
    {
        return true;
    }

    std::shared_ptr<Texture> texture = std::make_shared<Texture>();

    if (!GraphicsEngine::Get().LoadTexture(fullPath, *texture))
    {
        return false;
    }

    myTextures.emplace(normalizedPath, std::move(texture));

    LOG(FBXLog, Verbose, "Texture has loaded [{}]", normalizedPath.string());

    return true;
}

bool AssetLibrary::LoadMaterial(const std::filesystem::path& aPath)
{
    ZoneScopedN("Load Material");

    const std::filesystem::path normalizedPath = aPath.lexically_normal();
    const std::filesystem::path fullPath = myContentRoot / normalizedPath;

    if (myMaterials.contains(normalizedPath))
    {
        return true;
    }

    JsonData materialData = Json::Read(fullPath);

    if (!materialData.Valid())
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

        {
            std::filesystem::path albedoTexturePath = materialData.GetString("AlbedoTexture");
            std::filesystem::path normalTexturePath = materialData.GetString("NormalTexture");

            if (!LoadTexture(albedoTexturePath))
            {
                return false;
            }
            if (!LoadTexture(normalTexturePath))
            {
                return false;
            }

            instance->SetTexture(0, GetTexture(albedoTexturePath.string()));
            instance->SetTexture(1, GetTexture(normalTexturePath.string()));
        }

        myMaterials.emplace(normalizedPath, std::move(instance));

        LOG(FBXLog, Verbose, "Material instance has loaded [{}]", normalizedPath.string());

        return true;
    }

    MaterialDescription desc;
    desc.Domain = static_cast<MaterialDomain>(materialData.GetInt("MaterialDomain"));
    desc.ShadingModel = static_cast<ShadingModel>(materialData.GetInt("ShadingModel"));
    desc.BlendMode = static_cast<BlendMode>(materialData.GetInt("BlendMode"));
    desc.Name = materialData.GetString("Name");
    desc.MaterialShaderCode = myContentRoot / materialData.GetString("ShaderCode");

    std::shared_ptr<Material> material = std::make_shared<Material>();

    if (!GraphicsEngine::Get().CreateMaterial(desc, *material))
    {
        return false;
    }

    {
        std::filesystem::path albedoTexturePath = materialData.GetString("AlbedoTexture");
        std::filesystem::path normalTexturePath = materialData.GetString("NormalTexture");

        if (!LoadTexture(albedoTexturePath))
        {
            return false;
        }
        if (!LoadTexture(normalTexturePath))
        {
            return false;
        }

        material->SetTexture(0, GetTexture(albedoTexturePath.string()));
        material->SetTexture(1, GetTexture(normalTexturePath.string()));
    }

    myMaterials.emplace(normalizedPath, std::move(material));

    LOG(FBXLog, Verbose, "Material has loaded [{}]", normalizedPath.string());

    return true;
}

void AssetLibrary::InitUnitCube()
{
    std::vector<Vertex> meshVertices =
    {
        // Front
        { .Position = {-0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 0, 0,-1 }, .Tangent = { 1, 0, 0 } },
        { .Position = {-0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 0, 0,-1 }, .Tangent = { 1, 0, 0 } },
        { .Position = { 0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 0, 0,-1 }, .Tangent = { 1, 0, 0 } },
        { .Position = { 0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 0, 0,-1 }, .Tangent = { 1, 0, 0 } },

        // Back
        { .Position = {-0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 0, 0, 1 }, .Tangent = {-1, 0, 0 } },
        { .Position = {-0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 0, 0, 1 }, .Tangent = {-1, 0, 0 } },
        { .Position = { 0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 0, 0, 1 }, .Tangent = {-1, 0, 0 } },
        { .Position = { 0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 0, 0, 1 }, .Tangent = {-1, 0, 0 } },

        // Left
        { .Position = {-0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = {-1, 0, 0 }, .Tangent = { 0, 0, 1 } },
        { .Position = {-0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = {-1, 0, 0 }, .Tangent = { 0, 0, 1 } },
        { .Position = {-0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = {-1, 0, 0 }, .Tangent = { 0, 0, 1 } },
        { .Position = {-0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = {-1, 0, 0 }, .Tangent = { 0, 0, 1 } },

        // Right
        { .Position = { 0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 1, 0, 0 }, .Tangent = { 0, 0,-1 } },
        { .Position = { 0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 1, 0, 0 }, .Tangent = { 0, 0,-1 } },
        { .Position = { 0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 1, 0, 0 }, .Tangent = { 0, 0,-1 } },
        { .Position = { 0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 1, 0, 0 }, .Tangent = { 0, 0,-1 } },

        // Top
        { .Position = {-0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 0, 1, 0 }, .Tangent = { 1, 0, 0 } },
        { .Position = {-0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 0, 1, 0 }, .Tangent = { 1, 0, 0 } },
        { .Position = { 0.5f,  0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 0, 1, 0 }, .Tangent = { 1, 0, 0 } },
        { .Position = { 0.5f,  0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 0, 1, 0 }, .Tangent = { 1, 0, 0 } },

        // Bottom
        { .Position = {-0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 0,-1, 0 }, .Tangent = { 1, 0, 0 } },
        { .Position = {-0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 0,-1, 0 }, .Tangent = { 1, 0, 0 } },
        { .Position = { 0.5f, -0.5f, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 0,-1, 0 }, .Tangent = { 1, 0, 0 } },
        { .Position = { 0.5f, -0.5f,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 0,-1, 0 }, .Tangent = { 1, 0, 0 } },
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
    std::unique_ptr<CommonUtilities::Sphere<float>> boundingSphere = nullptr;

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    std::string name = "Unit Cube";

    mesh->Initialize(name, { element }, std::move(meshVertices), std::move(meshIndices), nullptr, std::move(boundingSphere));

    myMeshes.emplace("UnitCube", mesh);
}

void AssetLibrary::InitUnitPlane()
{
    std::vector<Vertex> meshVertices =
    {
        { .Position = {-0.5f, 0, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 0 }, .Normal = { 0, 1, 0 }, .Tangent = { 1, 0, 0 } },
        { .Position = {-0.5f, 0,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 1, 1 }, .Normal = { 0, 1, 0 }, .Tangent = { 1, 0, 0 } },
        { .Position = { 0.5f, 0, -0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 0 }, .Normal = { 0, 1, 0 }, .Tangent = { 1, 0, 0 } },
        { .Position = { 0.5f, 0,  0.5f, 1 }, .Color = { 1, 1, 1, 1 }, .UV0 = { 0, 1 }, .Normal = { 0, 1, 0 }, .Tangent = { 1, 0, 0 } }
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
    std::unique_ptr<CommonUtilities::Sphere<float>> boundingSphere = nullptr;
    
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    std::string name = "Unit Plane";

    mesh->Initialize(name, { element }, std::move(meshVertices), std::move(meshIndices), nullptr, std::move(boundingSphere));

    myMeshes.emplace("UnitPlane", mesh);
}

void AssetLibrary::InitUnitAxes()
{

    std::vector<Vertex> meshVertices =
    {
    };

    std::vector<unsigned> meshIndices =
    {
    };

    Mesh::Element element;

    element.NumVertices = static_cast<unsigned>(meshVertices.size());
    element.NumIndices = static_cast<unsigned>(meshIndices.size());

    // TODO: Calculate bounding box / sphere for unit axes
    std::unique_ptr<CommonUtilities::Sphere<float>> boundingSphere = nullptr;

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    std::string name = "Unit Axes";

    mesh->Initialize(name, { element }, std::move(meshVertices), std::move(meshIndices), nullptr, std::move(boundingSphere));

    myMeshes.emplace("UnitAxes", mesh);
}

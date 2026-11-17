#include "Core/Core.pch.h"
#include "MeshAsset.h"

#include <utility>
#include <memory>

#include "Graphics/Objects/GPU/Vertex.h"

#include "Graphics/Objects/Mesh/Mesh.h"
#include "Graphics/Objects/Mesh/Skeleton.h"

#include <CommonUtilities/Collision/Sphere.hpp>
#include <CommonUtilities/Collision/AABB3D.hpp>

#include <CommonUtilities/Math/Vector2.hpp>
#include <CommonUtilities/Math/Vector4.hpp>
#include <CommonUtilities/Math/Vector3.hpp>
#include <CommonUtilities/Math/Matrix4x4.hpp>

MeshAsset::MeshAsset()
{
    myMesh = std::make_unique<Mesh>();
}

MeshAsset::~MeshAsset() = default;

const Mesh& MeshAsset::GetMesh() const
{
    return *myMesh;
}

bool MeshAsset::Load(const std::filesystem::path& aPath)
{
    ZoneScopedN("Load Mesh")

        std::unique_ptr<TGA::FBX::Mesh> tgaMesh = std::make_unique<TGA::FBX::Mesh>();
    const TGA::FBX::FbxImportStatus& importStatus = TGA::FBX::Importer::LoadMesh(aPath, *tgaMesh);

    if (!importStatus)
    {
        const std::string pathString = aPath.string();

        switch (importStatus.Result)
        {
            case TGA::FBX::FbxImportResult::InternalError:
            {
                SPDLOG_ERROR("Could not load mesh [{}] : Internal FBX importer error!", pathString);
                break;
            }
            case TGA::FBX::FbxImportResult::NotInitialized:
            {
                SPDLOG_ERROR("Could not load mesh [{}] : FBX importer is not initialized!", pathString);
                break;
            }
            case TGA::FBX::FbxImportResult::FileNotFound:
            {
                SPDLOG_ERROR("Could not load mesh [{}] : File was not found!", pathString);
                break;
            }
            case TGA::FBX::FbxImportResult::NoMeshData:
            {
                SPDLOG_ERROR("Could not load mesh [{}] : No mesh data found in file!", pathString);
                break;
            }
            default:
            {
                SPDLOG_ERROR("Could not load mesh [{}] : Unknown error!", pathString);
                break;
            }
        }

        return false;
    }

    if (!importStatus.Warnings.empty())
    {
        const std::string pathString = aPath.string();

        for (const auto& [warning, amount] : importStatus.Warnings)
        {
            switch (warning)
            {
                case TGA::FBX::FbxImportWarning::TooManySkinWeights:
                {
                    SPDLOG_WARN("Mesh [{}] : Vertices had too many skin weights and were truncated!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::MissingSkinWeights:
                {
                    SPDLOG_WARN("Mesh [{}] : Vertices are missing skin weights!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::NoUVCoordinates:
                {
                    SPDLOG_WARN("Mesh [{}] : Surfaces have no UV coordinates!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::GenerateNormalsFailed:
                {
                    SPDLOG_WARN("Mesh [{}] : Failed to generate normals for surfaces!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::CannotTriangulate:
                {
                    SPDLOG_WARN("Mesh [{}] : Could not triangulate polygons!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::DuplicateBoneNames:
                {
                    SPDLOG_WARN("Mesh [{}] : Duplicate bone names detected!", pathString);
                    break;
                }
                default:
                {
                    SPDLOG_WARN("Mesh [{}] : Unknown warning!", pathString);
                    break;
                }
            }
        }
    }

    std::vector<Mesh::Element> elements;
    elements.reserve(tgaMesh->Elements.size());

    std::vector<Vertex> meshVertices;
    std::vector<unsigned> meshIndices;

    CommonUtilities::Vector3f meshMin(FLT_MAX, FLT_MAX, FLT_MAX);
    CommonUtilities::Vector3f meshMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

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

        CommonUtilities::Vector3f elemMin(FLT_MAX, FLT_MAX, FLT_MAX);
        CommonUtilities::Vector3f elemMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (size_t v = element.VertexOffset; v < meshVertices.size(); v++)
        {
            const auto& p = meshVertices[v].Position;
            elemMin.x = std::min(elemMin.x, p.x); elemMax.x = std::max(elemMax.x, p.x);
            elemMin.y = std::min(elemMin.y, p.y); elemMax.y = std::max(elemMax.y, p.y);
            elemMin.z = std::min(elemMin.z, p.z); elemMax.z = std::max(elemMax.z, p.z);
        }

        element.BoundingBox.InitWithMinAndMax(elemMin, elemMax);

        CommonUtilities::Vector3f center = element.BoundingBox.GetCenter();

        float radius = (center - elemMin).Length();

        element.BoundingSphere.InitWithCenterAndRadius(
            center,
            radius
        );

        meshMin.x = std::min(meshMin.x, elemMin.x); meshMax.x = std::max(meshMax.x, elemMax.x);
        meshMin.y = std::min(meshMin.y, elemMin.y); meshMax.y = std::max(meshMax.y, elemMax.y);
        meshMin.z = std::min(meshMin.z, elemMin.z); meshMax.z = std::max(meshMax.z, elemMax.z);

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

    CommonUtilities::AABB3D<float> boundingBox;
    boundingBox.InitWithMinAndMax(meshMin, meshMax);

    CommonUtilities::Vector3f center = boundingBox.GetCenter();

    float radius = (center - meshMin).Length();

    CommonUtilities::Sphere<float> boundingSphere;
    boundingSphere.InitWithCenterAndRadius(
        center,
        radius
    );

    myMesh->Initialize(aPath.filename().string(), std::move(elements), std::move(meshVertices), std::move(meshIndices), std::move(skeleton), boundingSphere, boundingBox);

    return true;
}

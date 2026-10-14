#include "GameFramework.pch.h"
#include "MeshAsset.h"

#include <utility>
#include <memory>

#include "GraphicsEngine/Objects/GPU/Vertex.h"

#include "GraphicsEngine/Objects/Mesh/Mesh.h"
#include "GraphicsEngine/Objects/Mesh/Skeleton.h"

#include <CommonUtilities/Collision/Sphere.hpp>
#include <CommonUtilities/Collision/AABB3D.hpp>

#include <CommonUtilities/Math/Vector2.hpp>
#include <CommonUtilities/Math/Vector4.hpp>
#include <CommonUtilities/Math/Vector3.hpp>
#include <CommonUtilities/Math/Matrix4x4.hpp>

#include "tgafbx/Importer.h"
#include "tgafbx/TgaFbxStructs.h"

#include <tracy/core/Tracy.hpp>

#include "Logger/Logger.h"

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
                spdlog::error("Could not load mesh [{}] : Internal FBX importer error!", pathString);
                break;
            }
            case TGA::FBX::FbxImportResult::NotInitialized:
            {
                spdlog::error("Could not load mesh [{}] : FBX importer is not initialized!", pathString);
                break;
            }
            case TGA::FBX::FbxImportResult::FileNotFound:
            {
                spdlog::error("Could not load mesh [{}] : File was not found!", pathString);
                break;
            }
            case TGA::FBX::FbxImportResult::NoMeshData:
            {
                spdlog::error("Could not load mesh [{}] : No mesh data found in file!", pathString);
                break;
            }
            default:
            {
                spdlog::error("Could not load mesh [{}] : Unknown error!", pathString);
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
                    spdlog::warn("Mesh [{}] : Vertices had too many skin weights and were truncated!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::MissingSkinWeights:
                {
                    spdlog::warn("Mesh [{}] : Vertices are missing skin weights!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::NoUVCoordinates:
                {
                    spdlog::warn("Mesh [{}] : Surfaces have no UV coordinates!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::GenerateNormalsFailed:
                {
                    spdlog::warn("Mesh [{}] : Failed to generate normals for surfaces!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::CannotTriangulate:
                {
                    spdlog::warn("Mesh [{}] : Could not triangulate polygons!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::DuplicateBoneNames:
                {
                    spdlog::warn("Mesh [{}] : Duplicate bone names detected!", pathString);
                    break;
                }
                default:
                {
                    spdlog::warn("Mesh [{}] : Unknown warning!", pathString);
                    break;
                }
            }
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

        element.BoundingSphere.InitWithCenterAndRadius(
            {
                tgaElement.BoxSphereBounds.Center[0],
                tgaElement.BoxSphereBounds.Center[1],
                tgaElement.BoxSphereBounds.Center[2]
            },
            tgaElement.BoxSphereBounds.Radius
        );

        element.BoundingBox.InitWithMinAndMax(
            {
                tgaElement.BoxBounds.Min[0],
                tgaElement.BoxBounds.Min[1],
                tgaElement.BoxBounds.Min[2]
            },
            {
                tgaElement.BoxBounds.Max[0],
                tgaElement.BoxBounds.Max[1],
                tgaElement.BoxBounds.Max[2]
            }
        );

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

    CommonUtilities::Sphere<float> boundingSphere;
    boundingSphere.InitWithCenterAndRadius(
        {
            tgaMesh->BoxSphereBounds.Center[0],
            tgaMesh->BoxSphereBounds.Center[1],
            tgaMesh->BoxSphereBounds.Center[2]
        },
        tgaMesh->BoxSphereBounds.Radius
    );

    CommonUtilities::AABB3D<float> boundingBox;
    boundingBox.InitWithMinAndMax(
        {
            tgaMesh->BoxBounds.Min[0],
            tgaMesh->BoxBounds.Min[1],
            tgaMesh->BoxBounds.Min[2]
        },
        {
            tgaMesh->BoxBounds.Max[0],
            tgaMesh->BoxBounds.Max[1],
            tgaMesh->BoxBounds.Max[2]
        }
    );

    myMesh->Initialize(aPath.filename().string(), std::move(elements), std::move(meshVertices), std::move(meshIndices), std::move(skeleton), boundingSphere, boundingBox);

    return true;
}

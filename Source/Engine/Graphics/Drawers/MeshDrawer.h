#pragma once

#include <vector>

#include "CommonUtilities/Math/Matrix4x4.hpp"

#include "Graphics/Materials/Material.h"
#include <Graphics/Objects/GPU/InstanceData.h>

class Mesh;

struct MeshDrawCall
{
    bool HasSkinning = false;
    const Mesh* Mesh = nullptr;

    const MaterialInterface* Materials[Material::MAX_MATERIAL_TEXTURE_COUNT] = {};

    CommonUtilities::Matrix4x4f Transform;

    const CommonUtilities::Matrix4x4f* JointTransforms = nullptr;

    unsigned int ObjectID = 0;

    std::vector<InstanceData> Instances;
};

class MeshDrawer
{
public:
	static MeshDrawer& Get();

    void Submit(const MeshDrawCall& aDrawCall);

    const std::vector<MeshDrawCall>& GetMeshDrawCalls() const;

    void BeginFrame();

private:
    std::vector<MeshDrawCall> myDrawCalls;
}; 
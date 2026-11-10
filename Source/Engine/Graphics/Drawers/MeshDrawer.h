#pragma once

#include <vector>

#include "CommonUtilities/Math/Matrix4x4.hpp"

#include "Graphics/Materials/Material.h"

class Mesh;

struct MeshDrawCall
{
    bool HasSkinning = false;
    const Mesh* Mesh = nullptr;
    const MaterialInterface* Materials[Material::MAX_MATERIAL_TEXTURE_COUNT] = {};
    CommonUtilities::Matrix4x4f Transform;
    const CommonUtilities::Matrix4x4f* JointTransforms = nullptr;
    unsigned int ObjectID;
};

class MeshDrawer
{
public:
	static MeshDrawer& Get();

    void Submit(const MeshDrawCall& aDrawCall);

    const std::vector<MeshDrawCall>& GetMeshDrawCalls() const;

    void Clear();

private:
    std::vector<MeshDrawCall> myDrawCalls;
}; 
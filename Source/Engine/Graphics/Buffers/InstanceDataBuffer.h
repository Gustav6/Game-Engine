#pragma once

#include "Graphics/Objects/GPU/InstanceData.h"

constexpr unsigned MAX_MESH_INSTANCES = 32;

struct InstanceDataBuffer
{
	InstanceData Instances[MAX_MESH_INSTANCES];
};

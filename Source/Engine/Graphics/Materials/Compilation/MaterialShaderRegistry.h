#pragma once

#include <unordered_map>
#include <filesystem>

#include "Graphics/Materials/Material.h"
#include <Graphics/Objects/GPU/Vertex.h>

struct ShaderProgramDescription
{
    std::filesystem::path VertexShader;
    std::filesystem::path PixelShader;

    VertexLayoutType VertexLayout = VertexLayoutType::None;

	Topology Topology = Topology::TriangleList;
};

class MaterialShaderRegistry
{
public:
    void Initialize(const std::filesystem::path& aShaderRoot);

    bool Resolve(const MaterialDescription& aDescription, ShaderProgramDescription& outProgram) const;

private:
    std::unordered_map<std::string, ShaderProgramDescription> myPrograms;
};
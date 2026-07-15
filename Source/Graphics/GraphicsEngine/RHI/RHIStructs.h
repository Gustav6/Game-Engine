#pragma once

struct Viewport
{
	float TopLeftX = 0;
	float TopLeftY = 0;
	float Width = 0;
	float Height = 0;
	float MinDepth = 0;
	float MaxDepth = 1;
};

enum class BufferType : uint8_t
{
	Unknown,
	VertexBuffer,
	IndexBuffer,
	ConstantBuffer,
};

enum class Topology : unsigned int
{
	Undefined = 0,
	TriangleList = 4,
};

struct VertexElementDescription
{
	std::string Semantic;
	unsigned int SemanticIndex = 0;
	unsigned int Format = 0;
};

enum PipelineStage_
{
	PipelineStage_None = 0,
	PipelineStage_InputAssembler = 1,
	PipelineStage_VertexShader = 2,
	PipelineStage_GeometryShader = 4,
	PipelineStage_ComputeShader = 8,
	PipelineStage_Rasterizer = 16,
	PipelineStage_PixelShader = 32,
	PipelineStage_OutputMerger = 64,
};

typedef int PipelineStages;

enum class ShaderType : unsigned int
{
	Unknown,
	VertexShader,
	PixelShader,
	GeometryShader,
	ComputeShader
};
#pragma once

#include "RHIStructs.h"
#include "wrl.h"

struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11GeometryShader;
struct ID3D11PixelShader;
struct ID3D11RasterizerState;
struct ID3D11BlendState;

enum class RasterizerState : unsigned int
{
	FillSolidCullBack,
	FillSolidCullFront,
};

struct PipelineStateDescription
{
	std::string Name;

	struct ShaderData
	{
		const uint8_t* ByteCode = nullptr;
		size_t ByteCodeSize = 0;
	};

	ShaderData VertexShader;
	ShaderData GeometryShader;
	ShaderData PixelShader;

	std::vector<VertexElementDescription> InputLayoutElements;
	Topology Topology = Topology::TriangleList;
	PSOBlendMode BlendMode = PSOBlendMode::Opaque;
	RasterizerState RasterizerState = RasterizerState::FillSolidCullBack;
};

class PipelineStateObject
{
	friend class RenderHardwareInterface;
	friend class GraphicsCommandList;

public:
	PipelineStateObject();
	~PipelineStateObject();

private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> myInputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> myVertexShader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> myGeometryShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> myPixelShader;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> myRasterizerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> myBlendState;

	std::string myName;
	Topology myTopology;
};


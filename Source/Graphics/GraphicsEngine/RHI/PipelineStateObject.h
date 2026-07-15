#pragma once

#include "RHIStructs.h"
#include "wrl.h"

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
	Microsoft::WRL::ComPtr<struct ID3D11InputLayout> myInputLayout;
	Microsoft::WRL::ComPtr<struct ID3D11VertexShader> myVertexShader;
	Microsoft::WRL::ComPtr<struct ID3D11GeometryShader> myGeometryShader;
	Microsoft::WRL::ComPtr<struct ID3D11PixelShader> myPixelShader;
	Microsoft::WRL::ComPtr<struct ID3D11RasterizerState> myRasterizerState;

	std::string myName;
	Topology myTopology;
};


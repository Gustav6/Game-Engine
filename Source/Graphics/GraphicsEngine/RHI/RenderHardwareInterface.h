#pragma once

#include <wrl.h>
#include <string_view>
#include <vector>

#include "CommonUtilities/Vector2.hpp"
#include "RHI/RHIStructs.h"

class Sampler;
struct SamplerDescription;
class Buffer;
struct Vertex;
class Texture;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11DeviceChild;
struct PipelineStateDescription;
class PipelineStateObject;
class GraphicsCommandList;

struct ID3DInclude;
class Shader;

class RenderHardwareInterface
{
public:
	RenderHardwareInterface();
	~RenderHardwareInterface();

	bool Initialize(HWND aWindowHandle, bool aEnableDebug, Texture& outBackBuffer, Texture& outDepthStencil);

	CommonUtilities::Vector2ui GetClientSize() const;

	bool CreateVertexBuffer(std::string_view aName, const std::vector<Vertex>& aVertexList, Buffer& outBuffer) const;
	bool CreateIndexBuffer(std::string_view aName, const std::vector<unsigned>& aIndexList, Buffer& outBuffer) const;
	bool CreateConstantBuffer(std::string_view aName, size_t aSize, Buffer& outBuffer) const;

	bool CreatePipelineStateObject(const PipelineStateDescription& aDescription, PipelineStateObject& outPSO) const;
	bool CreateCommandList(std::string_view aName, GraphicsCommandList& outCommandList) const;
	bool CreateTexture(std::string_view aName, const uint8_t* aByteCode, size_t aByteCodeSize, Texture& outTexture) const;
	bool CreateTexture(std::string_view aName, unsigned aWidth, unsigned aHeight, unsigned aNumMips, ResourceFormat aFormat, ResourceBindFlags aBindFlags, Texture& outTexture ) const;
	bool CreateSampler(const SamplerDescription& aDescription, Sampler& outSampler);
	bool CreateDepthStencil(std::string_view aName, unsigned int aWidth, unsigned int aHeight, Texture& outDepthStencil, bool aCube) const;

	void ExecuteCommandList(const GraphicsCommandList& aCommandList) const;

	void Present() const;

	bool CompileShader(ShaderType aShaderType, const std::filesystem::path& aPath, ID3DInclude* aIncludeHandler, bool aCompileDebug, Shader& outShader) const;

private:
	void SetObjectName(const Microsoft::WRL::ComPtr<ID3D11DeviceChild>& aObject, std::string_view aName) const;

	Microsoft::WRL::ComPtr<ID3D11Device> myDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> myContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mySwapChain;

	HWND myWindowHandle;
};
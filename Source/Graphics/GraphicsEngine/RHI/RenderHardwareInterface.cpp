#include "GraphicsEngine.pch.h"
#include "RenderHardwareInterface.h"

#include <d3dcompiler.h>
#include <fstream>

#include "StringHelpers.h"
#include "GraphicsEngine/Objects/GPU/Buffer.h"
#include "GraphicsEngine/Objects/GPU/Texture.h"
#include "GraphicsEngine/Objects/GPU/Vertex.h"
#include "GraphicsEngine/Objects/GPU/Shader.h"
#include "GraphicsEngine/Objects/GPU/Sampler.h"
#include "GraphicsEngine/Objects/GPU/GBuffer.h"

#include "PipelineStateObject.h"
#include "GraphicsCommandList.h"

#include "GraphicsEngine/InternalOperations/DDSTextureLoader11.h"

using namespace Microsoft::WRL;

RenderHardwareInterface::RenderHardwareInterface() 
{
	myWindowHandle = nullptr;
}

RenderHardwareInterface::~RenderHardwareInterface() = default;

bool RenderHardwareInterface::Initialize(HWND aWindowHandle, bool aEnableDebug, Texture& outBackBuffer, Texture& outDepthStencil, bool aEnableVsync)
{
	myWindowHandle = aWindowHandle;
	myVsyncEnabled = aEnableVsync;

	HRESULT result = E_FAIL;

	ComPtr<IDXGIFactory> dxFactory;

	result = CreateDXGIFactory(__uuidof(IDXGIFactory), &dxFactory);

	if (FAILED(result))
	{
		SPDLOG_CRITICAL("Failed to create DXGI Factory");
		return false;
	}

	SPDLOG_INFO("Initializing RHI...");

	ComPtr<IDXGIAdapter> tempAdapter;
	std::vector<ComPtr<IDXGIAdapter>> adapters;

	while (dxFactory->EnumAdapters(static_cast<unsigned int>(adapters.size()), &tempAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		adapters.emplace_back(tempAdapter);
	}

	ComPtr<IDXGIAdapter> selectedAdapter;

	DXGI_ADAPTER_DESC selectedAdapterDesc = {};

	for (const auto& adapter : adapters)
	{
		DXGI_ADAPTER_DESC desc = {};
		adapter->GetDesc(&desc);

		if (selectedAdapterDesc.DedicatedVideoMemory < desc.DedicatedVideoMemory)
		{
			selectedAdapter = adapter;
			selectedAdapterDesc = desc;
		}
	}

	const wchar_t* wideAdapterName = selectedAdapterDesc.Description;
	const std::string adapterName = str::wide_to_utf8(wideAdapterName);

	SPDLOG_INFO("Selected adapter is {}", adapterName);

	result = D3D11CreateDevice(
		selectedAdapter.Get(),
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL,
		aEnableDebug ? D3D11_CREATE_DEVICE_DEBUG : 0,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&myDevice,
		NULL,
		&myContext
	);

	if (FAILED(result))
	{
		SPDLOG_CRITICAL("Failed to create Device!");
		return false;
	}

	ComPtr<ID3D11Debug> deviceDebug;
	myDevice.As(&deviceDebug);

	if (deviceDebug)
	{
		ComPtr<ID3D11InfoQueue> infoQueue;
		deviceDebug->QueryInterface(IID_PPV_ARGS(&infoQueue));

		D3D11_MESSAGE_ID mask[] = 
		{
			D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS
		};

		D3D11_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(mask);
		filter.DenyList.pIDList = mask;
		
		infoQueue->AddStorageFilterEntries(&filter);
	}

	const std::string dxAdapterName = "Adapter";
	myDevice->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<unsigned int>(dxAdapterName.length() * sizeof(char)), dxAdapterName.data());

	const std::string contextName = "Context";
	myContext->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<unsigned int>(contextName.length() * sizeof(char)), contextName.data());

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.OutputWindow = aWindowHandle;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Flags = myVsyncEnabled? 0 : DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = true;

	result = dxFactory->CreateSwapChain(myDevice.Get(), &swapChainDesc, &mySwapChain);

	if (FAILED(result))
	{
		SPDLOG_CRITICAL("Failed to create Swap Chain!");
		return false;
	}

	const std::string swapChainName = "SwapChain";
	mySwapChain->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<unsigned int>(swapChainName.length() * sizeof(char)), swapChainName.data());

	ComPtr<ID3D11Texture2D> backBufferTexture;
	result = mySwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBufferTexture);

	if (FAILED(result))
	{
		SPDLOG_CRITICAL("Failed to fetch Back Buffer!");
		return false;
	}

	SetObjectName(backBufferTexture, "BackBuffer_T2D");

	result = myDevice->CreateRenderTargetView(backBufferTexture.Get(), nullptr, &outBackBuffer.myRTV);

	if (FAILED(result))
	{
		SPDLOG_CRITICAL("Failed to create Render Target View!");
		return false;
	}

	SetObjectName(outBackBuffer.myRTV, "BackBuffer_RTV");

	CommonUtilities::Vector2ui clientSize = GetClientSize();
	Viewport viewport = { 0, 0, static_cast<float>(clientSize.x), static_cast<float>(clientSize.y), 0, 1 };
	outBackBuffer.myViewport = viewport;

	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = clientSize.x;
	depthDesc.Height = clientSize.y;
	depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	depthDesc.ArraySize = 1;
	depthDesc.MipLevels = 1;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;

	ComPtr<ID3D11Texture2D> depthTexture;

	result = myDevice->CreateTexture2D(&depthDesc, nullptr, &depthTexture);

	if (FAILED(result))
	{
		SPDLOG_CRITICAL("Failed to create Depth Stencil!");
		return false;
	}

	SetObjectName(depthTexture, "DepthStencil_T2D");

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	result = myDevice->CreateDepthStencilView(depthTexture.Get(), &dsvDesc, &outDepthStencil.myDSV);

	if (FAILED(result))
	{
		SPDLOG_CRITICAL("Failed to create Depth Stencil View!");
		return false;
	}

	SetObjectName(outDepthStencil.myDSV, "DepthStencil_DSV");

	outDepthStencil.myViewport = viewport;

	SPDLOG_INFO("RHI Started!");
	return true;
}

CommonUtilities::Vector2ui RenderHardwareInterface::GetClientSize() const
{
	RECT clientRect = {};

	GetClientRect(myWindowHandle, &clientRect);

	const unsigned int width = clientRect.right - clientRect.left;
	const unsigned int height = clientRect.bottom - clientRect.top;

	return { width, height };
}

bool RenderHardwareInterface::CreateVertexBuffer(std::string_view aName, const std::vector<Vertex>& aVertexList, Buffer& outBuffer) const
{
	if (aVertexList.empty())
	{
		SPDLOG_ERROR("Failed to create vertex buffer for {}! Vertex list is empty", aName);
		return false;
	}

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.ByteWidth = static_cast<unsigned int>(sizeof(Vertex) * aVertexList.size());

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = aVertexList.data();

	const HRESULT result = myDevice->CreateBuffer(&desc, &data, &outBuffer.myBuffer);

	if (FAILED(result))
	{
		SPDLOG_ERROR("Failed to create vertex buffer for {}! Failed to create Buffer!", aName);
		return false;
	}

	std::string bufferName = std::format("{}_VX", aName);
	SetObjectName(outBuffer.myBuffer, bufferName);

	outBuffer.myName = aName;
	outBuffer.mySize = desc.ByteWidth;
	outBuffer.myStride = sizeof(Vertex);
	outBuffer.myType = BufferType::VertexBuffer;

	return true;
}

bool RenderHardwareInterface::CreateIndexBuffer(std::string_view aName, const std::vector<unsigned int>& aIndexList, Buffer& outBuffer) const
{
	if (aIndexList.empty())
	{
		SPDLOG_ERROR("Failed to create index buffer for {}! Index list is empty", aName);
		return false;
	}

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.ByteWidth = static_cast<unsigned int>(sizeof(unsigned int) * aIndexList.size());

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = aIndexList.data();

	const HRESULT result = myDevice->CreateBuffer(&desc, &data, &outBuffer.myBuffer);

	if (FAILED(result))
	{
		SPDLOG_ERROR("Failed to create index buffer for {}! Failed to create Buffer!", aName);
		return false;
	}

	std::string bufferName = std::format("{}_IX", aName);
	SetObjectName(outBuffer.myBuffer, bufferName);

	outBuffer.myName = aName;
	outBuffer.mySize = desc.ByteWidth;
	outBuffer.myStride = sizeof(unsigned int);
	outBuffer.myType = BufferType::IndexBuffer;

	return true;
}

bool RenderHardwareInterface::CreateConstantBuffer(std::string_view aName, size_t aSize, Buffer& outBuffer) const
{
	if (aSize > 65536)
	{
		SPDLOG_ERROR("Failed to create constant buffer {}! Size is greater than 64 KB!", aName);
		return false;
	}

	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = static_cast<unsigned int>(aSize);

	const HRESULT result = myDevice->CreateBuffer(&desc, nullptr, &outBuffer.myBuffer);

	if (FAILED(result))
	{
		SPDLOG_ERROR("Failed to create constant buffer {}!", aName);
		return false;
	}

	SetObjectName(outBuffer.myBuffer, aName);

	outBuffer.myName = aName;
	outBuffer.mySize = aSize;
	outBuffer.myStride = static_cast<unsigned int>(outBuffer.mySize);
	outBuffer.myType = BufferType::ConstantBuffer;

	return true;
}

bool RenderHardwareInterface::CreateDynamicVertexBuffer(std::string_view aName, unsigned int aMaxVertexCount, Buffer& outBuffer) const
{
	if (aMaxVertexCount == 0)
	{
		SPDLOG_ERROR("Failed to create dynamic vertex buffer for {}! Max vertex count is 0", aName);

		return false;
	}

	D3D11_BUFFER_DESC desc = {};

	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = static_cast<unsigned int>(sizeof(Vertex) * aMaxVertexCount);

	const HRESULT result = myDevice->CreateBuffer(&desc, nullptr, &outBuffer.myBuffer);

	if (FAILED(result))
	{
		SPDLOG_ERROR("Failed to create dynamic vertex buffer for {}!",	aName);

		return false;
	}

	const std::string bufferName = std::format("{}_Dynamic_VX", aName);

	SetObjectName(outBuffer.myBuffer, bufferName);

	outBuffer.myName = aName;
	outBuffer.mySize = desc.ByteWidth;
	outBuffer.myStride = sizeof(Vertex);
	outBuffer.myType = BufferType::VertexBuffer;

	return true;
}

bool RenderHardwareInterface::CreatePipelineStateObject(const PipelineStateDescription& aDescription, PipelineStateObject& outPSO) const
{
	ensure(!aDescription.Name.empty());

	bool hasErrored = false;

	if (aDescription.VertexShader.ByteCode)
	{
		ComPtr<ID3D11VertexShader> shader;
		const HRESULT result = myDevice->CreateVertexShader(aDescription.VertexShader.ByteCode, aDescription.VertexShader.ByteCodeSize, nullptr, &shader);

		if (FAILED(result))
		{
			SPDLOG_ERROR("Failed to create the vertex shader for the pipeline state object {}!", aDescription.Name);
			hasErrored = true;
		}
		else
		{
			const std::string shaderName = std::format("{}_VS", aDescription.Name);
			SetObjectName(shader, shaderName);
			outPSO.myVertexShader = shader;
		}
	}

	if (aDescription.GeometryShader.ByteCode)
	{
		ComPtr<ID3D11GeometryShader> shader;
		const HRESULT result = myDevice->CreateGeometryShader(aDescription.GeometryShader.ByteCode, aDescription.GeometryShader.ByteCodeSize, nullptr, &shader);

		if (FAILED(result))
		{
			SPDLOG_ERROR("Failed to create the geometry shader for the pipeline state object {}!", aDescription.Name);
			hasErrored = true;
		}
		else
		{
			const std::string shaderName = std::format("{}_GS", aDescription.Name);
			SetObjectName(shader, shaderName);
			outPSO.myGeometryShader = shader;
		}
	}

	if (aDescription.PixelShader.ByteCode)
	{
		ComPtr<ID3D11PixelShader> shader;
		const HRESULT result = myDevice->CreatePixelShader(aDescription.PixelShader.ByteCode, aDescription.PixelShader.ByteCodeSize, nullptr, &shader);

		if (FAILED(result))
		{
			SPDLOG_ERROR("Failed to create the pixel shader for the pipeline state object {}!", aDescription.Name);
			hasErrored = true;
		}
		else
		{
			const std::string shaderName = std::format("{}_PS", aDescription.Name);
			SetObjectName(shader, shaderName);
			outPSO.myPixelShader = shader;
		}
	}

	if (!aDescription.InputLayoutElements.empty())
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
		elements.reserve(aDescription.InputLayoutElements.size());

		for (const auto& desc : aDescription.InputLayoutElements)
		{
			D3D11_INPUT_ELEMENT_DESC element = {};
			element.SemanticName = desc.Semantic.c_str();
			element.SemanticIndex = desc.SemanticIndex;
			element.Format = static_cast<DXGI_FORMAT>(desc.Format);

			element.InputSlot = 0;
			element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			element.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			element.InstanceDataStepRate = 0;

			elements.emplace_back(element);
		}

		ComPtr<ID3D11InputLayout> inputLayout;
		const HRESULT result = myDevice->CreateInputLayout(
			elements.data(),
			static_cast<unsigned int>(elements.size()),
			aDescription.VertexShader.ByteCode,
			aDescription.VertexShader.ByteCodeSize,
			&inputLayout
		);

		if (FAILED(result))
		{
			SPDLOG_ERROR("Failed to create input layout for pipeline state object {}!", aDescription.Name);
			hasErrored = true;
		}
		else
		{
			const std::string ilName = std::format("{}_IL", aDescription.Name);
			SetObjectName(inputLayout, ilName);
			outPSO.myInputLayout = inputLayout;
		}
	}

	{
		D3D11_RASTERIZER_DESC desc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);

		switch (aDescription.RasterizerState)
		{
			case RasterizerState::FillSolidCullBack:
				desc.FillMode = D3D11_FILL_SOLID;
				desc.CullMode = D3D11_CULL_BACK;
				break;
			case RasterizerState::FillSolidCullFront:
				desc.FillMode = D3D11_FILL_SOLID;
				desc.CullMode = D3D11_CULL_FRONT;
				break;
			case RasterizerState::FillWireframeCullNone:
				desc.FillMode = D3D11_FILL_WIREFRAME;
				desc.CullMode = D3D11_CULL_NONE;
				break;
		}

		ComPtr<ID3D11RasterizerState> rasterizerState;
		const HRESULT result = myDevice->CreateRasterizerState(&desc, &rasterizerState);

		if (FAILED(result))
		{
			SPDLOG_ERROR("Failed to create rasterizer state for pipeline state object {}!", aDescription.Name);
			hasErrored = true;
		}
		else
		{
			const std::string rsName = std::format("{}_RS", aDescription.Name);
			SetObjectName(rasterizerState, rsName);
			outPSO.myRasterizerState = rasterizerState;
		}
	}

	switch (aDescription.BlendMode)
	{
		case BlendMode::Opaque:
		{
			outPSO.myBlendState = nullptr;
			break;
		}
		case BlendMode::Alpha:
		{
			D3D11_RENDER_TARGET_BLEND_DESC blend = {};

			blend.BlendEnable = true;

			blend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blend.BlendOp = D3D11_BLEND_OP_ADD;
			blend.SrcBlendAlpha = D3D11_BLEND_ONE; 
			blend.DestBlendAlpha = D3D11_BLEND_ZERO; 
			blend.BlendOpAlpha = D3D11_BLEND_OP_ADD;

			blend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			D3D11_BLEND_DESC desc = CD3D11_BLEND_DESC(D3D11_DEFAULT);

			desc.RenderTarget[0] = blend;

			ComPtr<ID3D11BlendState> blendState; 

			const HRESULT result = myDevice->CreateBlendState(&desc, &blendState);

			if (FAILED(result)) 
			{
				SPDLOG_ERROR("Failed to create aplha blend state for pipeline state object {}!", aDescription.Name);
				hasErrored = true;
			}
			else 
			{ 
				outPSO.myBlendState = blendState; 
			}

			break;
		}
		case BlendMode::Additive:
		{
			D3D11_RENDER_TARGET_BLEND_DESC blend = {}; 
			
			blend.BlendEnable = true; 

			blend.SrcBlend = D3D11_BLEND_ONE; 
			blend.DestBlend = D3D11_BLEND_ONE; 
			blend.BlendOp = D3D11_BLEND_OP_ADD; 
			blend.SrcBlendAlpha = D3D11_BLEND_ONE; 
			blend.DestBlendAlpha = D3D11_BLEND_ONE; 
			blend.BlendOpAlpha = D3D11_BLEND_OP_ADD; 
			blend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			D3D11_BLEND_DESC desc = CD3D11_BLEND_DESC(D3D11_DEFAULT);

			desc.RenderTarget[0] = blend;

			ComPtr<ID3D11BlendState> blendState;

			const HRESULT result = myDevice->CreateBlendState(&desc, &blendState);

			if (FAILED(result))
			{
				SPDLOG_ERROR("Failed to create additive blend state for pipeline state object {}!", aDescription.Name);
				hasErrored = true;
			}
			else
			{
				outPSO.myBlendState = blendState;
			}

			break;
		}
		default:
		{
			SPDLOG_ERROR("Unsupported blend mode provided for pipeline state object {}!", aDescription.Name);
			hasErrored = true;
			break;
		}
	}

	outPSO.myName = aDescription.Name;
	outPSO.myTopology = aDescription.Topology;

	return !hasErrored;
}

bool RenderHardwareInterface::CreateCommandList(std::string_view aName, GraphicsCommandList& outCommandList) const
{
	ensure(!outCommandList.myContext);

	const HRESULT result = myDevice->CreateDeferredContext(0, &outCommandList.myContext);

	if (FAILED(result))
	{
		SPDLOG_ERROR("Failed to create command list!");
		return false;
	}

	const std::string name = std::format("{}_CTXT", aName);
	SetObjectName(outCommandList.myContext, name);

	outCommandList.myName = aName;
	outCommandList.myContext->QueryInterface(IID_PPV_ARGS(&outCommandList.myUDA));

	return true;
}

bool RenderHardwareInterface::CreateTexture(std::string_view aName, const uint8_t* aByteCode, size_t aByteCodeSize, Texture& outTexture) const
{
	ensure(!aName.empty());

	ComPtr<ID3D11Resource> resource;
	ComPtr<ID3D11ShaderResourceView> srv;

	const HRESULT result = DirectX::CreateDDSTextureFromMemory(myDevice.Get(), aByteCode, aByteCodeSize, &resource, &srv);

	if (FAILED(result))
	{
		SPDLOG_ERROR("Failed to load Texture {}!", aName);
		return false;
	}

	SetObjectName(resource, aName);

	const std::string srvName = std::format("{}_SRV", aName);
	SetObjectName(srv, srvName);

	outTexture.mySRV = srv;
	outTexture.myName = aName;

	return true;
}

bool RenderHardwareInterface::CreateTexture(std::string_view aName, unsigned aWidth, unsigned aHeight, unsigned aNumMips, ResourceFormat aFormat, ResourceBindFlags aBindFlags, Texture& outTexture) const
{
	ensure(!aName.empty());

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = aWidth;
	desc.Height = aHeight;
	desc.MipLevels = aNumMips; // If this is 0 DX will create ALL possible Mips which is not always right!
	desc.ArraySize = 1;
	desc.Format = static_cast<DXGI_FORMAT>(aFormat);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = static_cast<UINT>(aBindFlags);
	desc.CPUAccessFlags = aBindFlags == ResourceBindFlag_Staging ? D3D11_CPU_ACCESS_READ : 0;
	desc.MiscFlags = aNumMips != 1 ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	ComPtr<ID3D11Texture2D> texture;
	HRESULT result = myDevice->CreateTexture2D(&desc, nullptr, &texture);
	if (FAILED(result))
	{
		SPDLOG_ERROR("Failed to create the texture {}!", aName);
		return false;
	}

	const std::string textureName = std::format("{}_T2D", aName);
	SetObjectName(texture, textureName);

	if (aBindFlags > 0)
	{
		if (aBindFlags & ResourceBindFlag_RenderTarget)
		{
			result = myDevice->CreateRenderTargetView(texture.Get(), nullptr, &outTexture.myRTV);
			if (FAILED(result))
			{
				SPDLOG_ERROR("Failed to create the render target view for texture {}!", aName);
				return false;
			}

			const std::string resourceName = std::format("{}_RTV", aName);
			SetObjectName(outTexture.myRTV, resourceName);
		}

		if (aBindFlags & ResourceBindFlag_ShaderResource)
		{
			result = myDevice->CreateShaderResourceView(texture.Get(), nullptr, &outTexture.mySRV);
			if (FAILED(result))
			{
				SPDLOG_ERROR("Failed to create the shader resource view for texture {}!", aName);
				return false;
			}

			const std::string resourceName = std::format("{}_SRV", aName);
			SetObjectName(outTexture.mySRV, resourceName);
		}

		// This is not required for the course but allows you to use this function to create
        // ShadowMaps as well if you wish!
		if (aBindFlags & ResourceBindFlag_DepthStencil)
		{
			result = myDevice->CreateDepthStencilView(texture.Get(), nullptr, &outTexture.myDSV);
			if (FAILED(result))
			{
				SPDLOG_ERROR("Failed to create the depth stencil view for texture {}!", aName);
				return false;
			}

			const std::string resourceName = std::format("{}_DSV", aName);
			SetObjectName(outTexture.myDSV, resourceName);
		}
	}

	outTexture.myViewport.TopLeftX = 0;
	outTexture.myViewport.TopLeftY = 0;
	outTexture.myViewport.Width = static_cast<float>(aWidth);
	outTexture.myViewport.Height = static_cast<float>(aHeight);
	outTexture.myViewport.MinDepth = 0;
	outTexture.myViewport.MaxDepth = 1;

	return true;
}

bool RenderHardwareInterface::CreateSampler(const SamplerDescription& aDescription, Sampler& outSampler)
{
	ensure(!aDescription.Name.empty());

	D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(D3D11_DEFAULT);
	samplerDesc.Filter = static_cast<D3D11_FILTER>(aDescription.FilterMode);
	samplerDesc.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(aDescription.AddressMode);
	samplerDesc.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(aDescription.AddressMode);
	samplerDesc.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(aDescription.AddressMode);
	samplerDesc.ComparisonFunc = static_cast<D3D11_COMPARISON_FUNC>(aDescription.ComparisonFunction);
	samplerDesc.BorderColor[0] = aDescription.BorderColor.x;
	samplerDesc.BorderColor[1] = aDescription.BorderColor.y;
	samplerDesc.BorderColor[2] = aDescription.BorderColor.z;
	samplerDesc.BorderColor[3] = aDescription.BorderColor.w;

	const HRESULT result = myDevice->CreateSamplerState(&samplerDesc, &outSampler.mySampler);
	if (FAILED(result))
	{
		SPDLOG_ERROR("Failed to create Sampler State {}!", aDescription.Name);
		return false;
	}

	SetObjectName(outSampler.mySampler, aDescription.Name);
	outSampler.myDescription = aDescription;

	return true;
}

bool RenderHardwareInterface::CreateDepthStencil(std::string_view aName, unsigned int aWidth, unsigned int aHeight, Texture& outDepthStencil, bool aCube) const
{
	ensure(!aName.empty());

	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = aWidth;
	depthDesc.Height = aHeight;
	depthDesc.Format = DXGI_FORMAT_R32_TYPELESS; // DXGI_FORMAT_R32G8X24_TYPELESS
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = aCube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = aCube ? 6 : 1;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;

	HRESULT result = S_OK;

	ComPtr<ID3D11Texture2D> depthTexture;
	result = myDevice->CreateTexture2D(&depthDesc, nullptr, &depthTexture);
	if (FAILED(result))
	{
		SPDLOG_ERROR("Failed to create depth stencil!");
		return false;
	}

	const std::string textureName = std::format("{}_T2D", aName);
	SetObjectName(depthTexture, textureName);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // DXGI_FORMAT_D32_FLOAT_S8X24_UINT
	dsvDesc.ViewDimension = aCube ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2DArray.ArraySize = aCube ? 6 : 1;

	result = myDevice->CreateDepthStencilView(depthTexture.Get(), &dsvDesc, &outDepthStencil.myDSV);
	if (FAILED(result))
	{
		SPDLOG_ERROR("Failed to create depth stencil view!");
		return false;
	}

	const std::string dsvName = std::format("{}_DSV", aName);
	SetObjectName(outDepthStencil.myDSV, dsvName);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS
	srvDesc.ViewDimension = aCube ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = depthDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	result = myDevice->CreateShaderResourceView(depthTexture.Get(), &srvDesc, &outDepthStencil.mySRV);
	if (FAILED(result))
	{
		SPDLOG_ERROR("Failed to create depth stencil shader resource view!");
		return false;
	}

	const std::string srvName = std::format("{}_SRV", aName);
	SetObjectName(outDepthStencil.mySRV, srvName);

	outDepthStencil.myViewport = { 0, 0, static_cast<float>(aWidth), static_cast<float>(aHeight), 0, 1 };
	outDepthStencil.myName = aName;

	return true;
}

bool RenderHardwareInterface::CreateGBuffer(std::string_view aName, unsigned aWidth, unsigned aHeight, GBuffer& outGBuffer) const
{
	ensure(!aName.empty());

	// To make texture creation easier and keep a debug name available.
	struct GBufferTextureInfo
	{
		std::string Name;
		DXGI_FORMAT Format;
	};

	// This order matters! Make SURE you always keep the GBuffer order
	// the same in all places. I.e. both here and in the shader code!
	// If there's a mismatch you will get very strange behavior.
	std::vector<GBufferTextureInfo> textureInfos = {
		{ "Albedo", DXGI_FORMAT_R8G8B8A8_UNORM },
		{ "Normal", DXGI_FORMAT_R16G16B16A16_FLOAT },
		{ "Surface", DXGI_FORMAT_R8G8B8A8_UNORM },
		{ "Emission", DXGI_FORMAT_R16G16B16A16_FLOAT },
		{ "WorldPosition", DXGI_FORMAT_R32G32B32A32_FLOAT }
	};

	// Base texture description.
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = aWidth;
	desc.Height = aHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	// desc.Format - This will be set in the loop below.
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	outGBuffer.myName = aName;

	for (size_t t = 0; t < textureInfos.size(); ++t)
	{
		const GBufferTextureInfo& info = textureInfos[t];
		desc.Format = info.Format;

		ComPtr<ID3D11Texture2D> texture;
		HRESULT result = myDevice->CreateTexture2D(&desc, nullptr, &texture);
		if (FAILED(result))
		{
			SPDLOG_ERROR("Failed to create the GBuffer {} channel!", aName, info.Name);
			return false;
		}
		const std::string textureName = std::format("{}_{}_T2D", aName, info.Name);
		SetObjectName(texture, textureName);

		ComPtr<ID3D11RenderTargetView> rtv;
		result = myDevice->CreateRenderTargetView(texture.Get(), nullptr, &rtv);
		if (FAILED(result))
		{
			SPDLOG_ERROR("Failed to create the GBuffer {} channel!", aName, info.Name);
			return false;
		}

		const std::string rtvName = std::format("{}_{}_RTV", aName, info.Name);
		SetObjectName(rtv, rtvName);

		ComPtr<ID3D11ShaderResourceView> srv;
		result = myDevice->CreateShaderResourceView(texture.Get(), nullptr, &srv);
		if (FAILED(result))
		{
			SPDLOG_ERROR("Failed to create the GBuffer {} channel!", aName, info.Name);
			return false;
		}

		const std::string srvName = std::format("{}_{}_SRV", aName, info.Name);
		SetObjectName(srv, srvName);

		outGBuffer.myTextures[t].myRTV = rtv;
		outGBuffer.myTextures[t].mySRV = srv;
		outGBuffer.myTextures[t].myName = info.Name;
		outGBuffer.myTextures[t].myViewport.TopLeftX = 0;
		outGBuffer.myTextures[t].myViewport.TopLeftY = 0;
		outGBuffer.myTextures[t].myViewport.Width = static_cast<float>(aWidth);
		outGBuffer.myTextures[t].myViewport.Height = static_cast<float>(aHeight);
		outGBuffer.myTextures[t].myViewport.MinDepth = 0;
		outGBuffer.myTextures[t].myViewport.MaxDepth = 1;
	}

	return true;
}

void RenderHardwareInterface::ExecuteCommandList(const GraphicsCommandList& aCommandList) const
{
	ensure(aCommandList.IsReadyForExecution());

	myContext->ExecuteCommandList(aCommandList.myCommandList.Get(), false);
}

void RenderHardwareInterface::Present() const
{
	if (myVsyncEnabled)
	{
		mySwapChain->Present(1, 0);
	}
	else
	{
		mySwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	}
}

bool RenderHardwareInterface::CompileShader(ShaderType aShaderType, const std::filesystem::path& aPath, ID3DInclude* aIncludeHandler, bool aCompileDebug, Shader& outShader) const
{
	std::ifstream codeFile(aPath, std::ios::binary);
	std::ostringstream codeFileStream;
	codeFileStream << codeFile.rdbuf();
	codeFile.close();
	const std::string shaderSource = codeFileStream.str();

	if (shaderSource.empty())
	{
		SPDLOG_ERROR("Unable to load shader {}!", aPath.string());
		return false;
	}

	std::string shaderTarget(6, ' ');

	switch (aShaderType)
	{
		case ShaderType::VertexShader:
			shaderTarget = "vs_5_0";
			break;
		case ShaderType::PixelShader:
			shaderTarget = "ps_5_0";
			break;
		case ShaderType::GeometryShader:
			shaderTarget = "gs_5_0";
			break;
		case ShaderType::ComputeShader:
			shaderTarget = "cs_5_0";
			break;
		default:
			SPDLOG_ERROR("Unknown shader type specified when loading {}!", aPath.string());
			return false;
	}

	std::vector<D3D_SHADER_MACRO> shaderMacros;
	unsigned int shaderFlags = D3DCOMPILE_WARNINGS_ARE_ERRORS;

	if (aCompileDebug)
	{
		shaderMacros.emplace_back(D3D_SHADER_MACRO{ .Name = "_DEBUG", .Definition = "1" });
		shaderFlags |= D3DCOMPILE_DEBUG;
	}

	shaderMacros.emplace_back(D3D_SHADER_MACRO{ .Name = nullptr, .Definition = nullptr });

	ComPtr<ID3DBlob> compiledShader;
	ComPtr<ID3DBlob> errorMessages;

	const HRESULT result = D3DCompile(
		shaderSource.data(),
		shaderSource.size(),
		aPath.string().c_str(),
		shaderMacros.data(),
		aIncludeHandler,
		"main",
		shaderTarget.c_str(),
		shaderFlags,
		0,
		&compiledShader,
		&errorMessages
	);

	if (FAILED(result))
	{
		const std::string message = static_cast<const char*>(errorMessages->GetBufferPointer());
		SPDLOG_ERROR("Shader compilation failed!");
		SPDLOG_ERROR("Error: {}", message);
		return false;
	}

	outShader.myBlob = compiledShader;
	outShader.myType = aShaderType;

	return true;
}

ID3D11Device* RenderHardwareInterface::GetDevice() const
{
	return myDevice.Get();
}

ID3D11DeviceContext* RenderHardwareInterface::GetContext() const
{
	return myContext.Get();
}

void RenderHardwareInterface::BindRenderTarget(const Texture* aTexture)
{
	ID3D11RenderTargetView* rtv = aTexture->myRTV.Get();
	myContext->OMSetRenderTargets(1, &rtv, nullptr);
}

void RenderHardwareInterface::SetObjectName(const Microsoft::WRL::ComPtr<ID3D11DeviceChild>& aObject, std::string_view aName) const
{
	if (aObject)
	{
		aObject->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<unsigned int>(aName.length() * sizeof(char)), aName.data());
	}
}

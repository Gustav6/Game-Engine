#include "GraphicsEngine.pch.h"
#include "GraphicsCommandList.h"

#include "Objects/Sampler.h"
#include "Objects/Texture.h"
#include "Objects/Buffer.h"

#include "PipelineStateObject.h"

DECLARE_LOG_CATEGORY_WITH_NAME(CmdLog, CommandList, Verbose);

DEFINE_LOG_CATEGORY(CmdLog);

GraphicsCommandList::GraphicsCommandList() = default;
GraphicsCommandList::~GraphicsCommandList() = default;

GraphicsCommandList::GraphicsCommandList(GraphicsCommandList&& aOther) noexcept
{
	*this = std::move(aOther);
}

GraphicsCommandList& GraphicsCommandList::operator=(GraphicsCommandList && aOther) noexcept
{
	if (this != &aOther)
	{
		myContext.Reset();
		myCommandList.Reset();
		myUDA.Reset();

		aOther.myContext.Swap(myContext);
		aOther.myCommandList.Swap(myCommandList);
		aOther.myUDA.Swap(myUDA);
	}

	return *this;
}

void GraphicsCommandList::FinishCommandList()
{
	ensure(!IsReadyForExecution());

	myContext->FinishCommandList(false, &myCommandList);
	const std::string name = std::format("{}_CMD", myName);
	myCommandList->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<unsigned>(sizeof(char) * name.length()), name.data());
}

void GraphicsCommandList::ResetCommandList()
{
	myCommandList.Reset();
}

void GraphicsCommandList::ClearRenderTarget(const Texture& aTarget) const
{
	ensure(!IsReadyForExecution());

	float clearColor[4] = { 0, 0, 0, 0 };
	myContext->ClearRenderTargetView(aTarget.myRTV.Get(), clearColor);
}

void GraphicsCommandList::ClearDepthStencil(const Texture& aTarget) const
{
	ensure(!IsReadyForExecution());

	myContext->ClearDepthStencilView(aTarget.myDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void GraphicsCommandList::SetRenderTarget(const Texture* aTexture, const Texture* aDepthStencil) const
{
	ensure(!IsReadyForExecution());

	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11DepthStencilView* dsv = nullptr;
	D3D11_VIEWPORT viewport = { 0, 0, 0, 0, 0, 1 };

	if (aTexture != nullptr)
	{
		rtv = aTexture->myRTV.Get();
		memcpy_s(&viewport, sizeof(D3D11_VIEWPORT), &aTexture->myViewport, sizeof(Viewport));
	}

	if (aDepthStencil != nullptr)
	{
		dsv = aDepthStencil->myDSV.Get();

		if (aTexture == nullptr)
		{
			memcpy_s(&viewport, sizeof(D3D11_VIEWPORT), &aDepthStencil->myViewport, sizeof(Viewport));
		}
	}

	myContext->OMSetRenderTargets(1, &rtv, dsv);
	myContext->RSSetViewports(1, &viewport);
}


bool GraphicsCommandList::UpdateConstantBuffer(const Buffer& aConstantBuffer, const void* aBufferData, size_t aBufferDataSize) const
{
	ensure(!IsReadyForExecution());

	if (!aConstantBuffer.IsValid() || aConstantBuffer.myType != BufferType::ConstantBuffer)
	{
		LOG(CmdLog, Error, "Failed to update constant buffer! Buffer is either null or invalid type!");
		return false;
	}

	if (aBufferDataSize > aConstantBuffer.mySize)
	{
		LOG(CmdLog, Error, "Failed to update constant buffer {}! Data provided is larger than buffer capacity!", aConstantBuffer.myName);
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE resource = {};

	const HRESULT result = myContext->Map(aConstantBuffer.myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

	if (FAILED(result))
	{
		LOG(CmdLog, Error, "Failed to update constant buffer {}! Failed to map buffer!", aConstantBuffer.myName);
		return false;
	}

	memcpy_s(resource.pData, aConstantBuffer.mySize, aBufferData, aBufferDataSize);
	myContext->Unmap(aConstantBuffer.myBuffer.Get(), 0);

	return true;
}

void GraphicsCommandList::SetVertexBuffer(const Buffer* aBuffer) const
{
	ensure(!IsReadyForExecution());

	constexpr unsigned offset = 0;

	if (aBuffer)
	{
		myContext->IASetVertexBuffers(0, 1, aBuffer->myBuffer.GetAddressOf(), &aBuffer->myStride, &offset);
	}
	else
	{
		const unsigned stride = 0;
		ID3D11Buffer* buffer = nullptr;
		myContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	}
}

void GraphicsCommandList::SetIndexBuffer(const Buffer* aBuffer) const
{
	ensure(!IsReadyForExecution());

	if (aBuffer)
	{
		myContext->IASetIndexBuffer(aBuffer->myBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}
	else
	{
		ID3D11Buffer* buffer = nullptr;
		myContext->IASetIndexBuffer(buffer, DXGI_FORMAT_UNKNOWN, 0);
	}
}

void GraphicsCommandList::SetConstantBuffer(const Buffer* aBuffer, unsigned aSlot, PipelineStages aStages) const
{
	ensure(!IsReadyForExecution());

	ID3D11Buffer* buffer = nullptr;

	if (aBuffer)
	{
		buffer = aBuffer->myBuffer.Get();
	}

	if (aStages & PipelineStage_VertexShader)
	{
		myContext->VSSetConstantBuffers(aSlot, 1, &buffer);
	}
	if (aStages & PipelineStage_PixelShader)
	{
		myContext->PSSetConstantBuffers(aSlot, 1, &buffer);
	}
	if (aStages & PipelineStage_GeometryShader)
	{
		myContext->GSSetConstantBuffers(aSlot, 1, &buffer);
	}
}

void GraphicsCommandList::SetPipelineStateObject(const PipelineStateObject& aPSO, const std::string_view aMarkerLabel) const
{
	ensure(!IsReadyForExecution());

	if (IsOverrideActive(PipelineStage_VertexShader))
	{
		myContext->VSSetShader(myOverridePipelineState->myVertexShader.Get(), nullptr, 0);
	}
	else 
	{
		myContext->VSSetShader(aPSO.myVertexShader.Get(), nullptr, 0);
	}
		
	if (IsOverrideActive(PipelineStage_GeometryShader))
	{
		myContext->GSSetShader(myOverridePipelineState->myGeometryShader.Get(), nullptr, 0);
	}
	else
	{
		myContext->GSSetShader(aPSO.myGeometryShader.Get(), nullptr, 0);
	}

	if (IsOverrideActive(PipelineStage_Rasterizer))
	{
		myContext->RSSetState(myOverridePipelineState->myRasterizerState.Get());
	}
	else
	{
		myContext->RSSetState(aPSO.myRasterizerState.Get());
	}

	if (IsOverrideActive(PipelineStage_PixelShader))
	{
		myContext->PSSetShader(myOverridePipelineState->myPixelShader.Get(), nullptr, 0);
	}
	else
	{
		myContext->PSSetShader(aPSO.myPixelShader.Get(), nullptr, 0);
	}

	myContext->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(aPSO.myTopology));
	myContext->IASetInputLayout(aPSO.myInputLayout.Get());

	if (aMarkerLabel.empty())
	{
		const std::string message = std::format("Change Pipeline state - {}", aPSO.myName);

		SetMarker(message);
	}
	else
	{
		SetMarker(aMarkerLabel);
	}
}

void GraphicsCommandList::SetShaderResource(const Texture* aTexture, unsigned int aSlot, PipelineStages aStages) const
{
	if (aTexture == nullptr)
	{
		LOG(CmdLog, Error, "Tried to bind a shader resource which was null!");
		return;
	}

	ID3D11ShaderResourceView* srv = aTexture->mySRV.Get();

	if (srv == nullptr)
	{
		LOG(CmdLog, Error, "Tried to bind a shader resource with a null SRV!");
		return;
	}

	if (aStages & PipelineStage_VertexShader)
	{
		myContext->VSSetShaderResources(aSlot, 1, &srv);
	}
	if (aStages & PipelineStage_PixelShader)
	{
		myContext->PSSetShaderResources(aSlot, 1, &srv);
	}
	if (aStages & PipelineStage_GeometryShader)
	{
		myContext->GSSetShaderResources(aSlot, 1, &srv);
	}
}

void GraphicsCommandList::SetShaderResources(const Texture* const* aResourcesList, size_t aNumOfResources, unsigned int aStartSlot, PipelineStages aStages) const
{
	std::vector<ID3D11ShaderResourceView*> srvs(aNumOfResources);

	for (size_t t = 0; t < aNumOfResources; t++)
	{
		if (const Texture* currentTexture = aResourcesList[t])
		{
			srvs[t] = currentTexture->mySRV.Get();
		}
	}

	if (aStages & PipelineStage_VertexShader)
	{
		myContext->VSSetShaderResources(aStartSlot, static_cast<unsigned>(srvs.size()), srvs.data());
	}
	if (aStages & PipelineStage_PixelShader)
	{
		myContext->PSSetShaderResources(aStartSlot, static_cast<unsigned>(srvs.size()), srvs.data());
	}
	if (aStages & PipelineStage_GeometryShader)
	{
		myContext->GSSetShaderResources(aStartSlot, static_cast<unsigned>(srvs.size()), srvs.data());
	}
}

void GraphicsCommandList::SetShaderSamplers(const Sampler* const* aSamplerList, size_t aNumOfSamplers, unsigned int aStartSlot, PipelineStages aStages) const
{
	std::vector<ID3D11SamplerState*> samplers(aNumOfSamplers);

	for (size_t s = 0; s < aNumOfSamplers; s++)
	{
		if (const Sampler* currentSampler = aSamplerList[s])
		{
			samplers[s] = currentSampler->mySampler.Get();
		}
	}

	if (aStages & PipelineStage_VertexShader)
	{
		myContext->VSSetSamplers(aStartSlot, static_cast<unsigned>(samplers.size()), samplers.data());
	}
	if (aStages & PipelineStage_PixelShader)
	{
		myContext->PSSetSamplers(aStartSlot, static_cast<unsigned>(samplers.size()), samplers.data());
	}
	if (aStages & PipelineStage_GeometryShader)
	{
		myContext->GSSetSamplers(aStartSlot, static_cast<unsigned>(samplers.size()), samplers.data());
	}
}

void GraphicsCommandList::Draw(unsigned aNumVertices) const
{
	ensure(!IsReadyForExecution());

	myContext->Draw(aNumVertices, 0);
}

void GraphicsCommandList::DrawIndexed(unsigned aIndexCount, unsigned aIndexOffset) const
{
	ensure(!IsReadyForExecution());

	myContext->DrawIndexed(aIndexCount, aIndexOffset, 0);
}

void GraphicsCommandList::SetMarker(std::string_view aMarker) const
{
	ensure(!IsReadyForExecution());

	const std::wstring wideMarker = str::acp_to_wide_ascii(aMarker.data());

	myUDA->SetMarker(wideMarker.c_str());
}

void GraphicsCommandList::BeginEvent(std::string_view aEvent) const
{
	ensure(!IsReadyForExecution());

	const std::wstring wideEvent = str::acp_to_wide_ascii(aEvent.data());

	myUDA->BeginEvent(wideEvent.c_str());
}

void GraphicsCommandList::EndEvent() const
{
	ensure(!IsReadyForExecution());

	myUDA->EndEvent();
}

void GraphicsCommandList::SetOverridePipelineState(const PipelineStateObject& aOverridePSO, PipelineStages aOverrideStages)
{
	myOverridePipelineState = std::make_unique<PipelineStateObject>(aOverridePSO);
	myCurrentOverrides = aOverrideStages;
}

void GraphicsCommandList::ChangeActiveOverrides(PipelineStages aOverrideStages)
{
	myCurrentOverrides = aOverrideStages;
}

bool GraphicsCommandList::IsOverrideActive(PipelineStages aOverride) const
{
	return (myCurrentOverrides & aOverride);
}

void GraphicsCommandList::ClearOverridePipelineState()
{
	myOverridePipelineState = nullptr;
	myCurrentOverrides = PipelineStage_None;
}

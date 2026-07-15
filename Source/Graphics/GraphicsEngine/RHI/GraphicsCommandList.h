#pragma once

#include <wrl.h>
#include <string>
#include <string_view>

#include "GraphicsEngine/RHI/RHIStructs.h"

class Buffer;
class Texture;
class Sampler;
class PipelineStateObject;

class GraphicsCommandList
{
	friend class RenderHardwareInterface;

public:
	GraphicsCommandList();
	~GraphicsCommandList();

	GraphicsCommandList(const GraphicsCommandList&) = delete;
	GraphicsCommandList& operator=(const GraphicsCommandList&) = delete;

	GraphicsCommandList(GraphicsCommandList&& aOther) noexcept;
	GraphicsCommandList& operator=(GraphicsCommandList&& aOther) noexcept;

	void FinishCommandList();
	void ResetCommandList();

	bool IsReadyForExecution() const { return myCommandList; }

	void ClearRenderTarget(const Texture& aTarget) const;
	void ClearDepthStencil(const Texture& aTarget) const;
	void SetRenderTarget(const Texture* aTarget, const Texture* aDepthStencil) const;

	bool UpdateConstantBuffer(const Buffer& aConstantBuffer, const void* aBufferData, size_t aBufferDataSize) const;

	void SetVertexBuffer(const Buffer* aBuffer) const;
	void SetIndexBuffer(const Buffer* aBuffer) const;
	void SetConstantBuffer(const Buffer* aBuffer, unsigned aSlot, PipelineStages aStages) const;
	void SetPipelineStateObject(const PipelineStateObject& aPSO, const std::string_view aMarkerLabel = {}) const;

	void SetShaderResources(const Texture* const* aResourcesList, size_t aNumOfResources, unsigned int aStartSlot, PipelineStages aStages) const;
	void SetShaderSamplers(const Sampler* const* aSamplerList, size_t aNumOfSamplers, unsigned int aStartSlot, PipelineStages aStages) const;

	void Draw(unsigned aNumVertices) const;
	void DrawIndexed(unsigned aIndexCount, unsigned aIndexOffset) const;

	void SetMarker(std::string_view aMarker) const;
	void BeginEvent(std::string_view aEvent) const;
	void EndEvent() const;

	void SetOverridePipelineState(const PipelineStateObject& aOverridePSO, PipelineStages aOverrideStages);
	void ChangeActiveOverrides(PipelineStages aOverrideStages);
	bool IsOverrideActive(PipelineStages aOverride) const;
	void ClearOverridePipelineState();

private:
	std::unique_ptr<PipelineStateObject> myOverridePipelineState;
	PipelineStages myCurrentOverrides = PipelineStage_None;

	std::string myName;

	Microsoft::WRL::ComPtr<struct ID3D11DeviceContext> myContext;
	Microsoft::WRL::ComPtr<struct ID3D11CommandList> myCommandList;
	Microsoft::WRL::ComPtr<struct ID3DUserDefinedAnnotation> myUDA;
};


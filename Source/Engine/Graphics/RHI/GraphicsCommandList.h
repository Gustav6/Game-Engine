#pragma once

#include <wrl.h>
#include <string>
#include <string_view>

#include "Graphics/RHI/RHIStructs.h"

class Buffer;
class Texture;
class Sampler;
class PipelineStateObject;
struct Vertex;

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
	void ClearRenderTargets(const Texture* aTargetList, unsigned aNumTargets) const;
	void ClearDepthStencil(const Texture& aTarget) const;

	void SetRenderTarget(const Texture* aTarget, const Texture* aDepthStencil) const;
	void SetRenderTargets(const Texture* aTargetList, unsigned aNumTargets, const Texture* aDepthStencil) const;

	bool UpdateConstantBuffer(const Buffer& aConstantBuffer, const void* aBufferData, size_t aBufferDataSize) const;
	bool UpdateDynamicVertexBuffer(const Buffer& aBuffer, const void* aBufferData, size_t aBufferDataSize) const;

	void SetVertexBuffer(const Buffer* aBuffer) const;
	void SetIndexBuffer(const Buffer* aBuffer) const;
	void SetConstantBuffer(const Buffer* aBuffer, unsigned aSlot, PipelineStages aStages) const;
	void SetPipelineStateObject(const PipelineStateObject& aPSO, const std::string_view aMarkerLabel = {}) const;

	void SetShaderResource(const Texture* aTexture, unsigned int aSlot, PipelineStages aStages) const;
	
	void SetShaderResources(const Buffer* const* aResourceList, size_t aNumResources, unsigned aStartSlot, PipelineStages aStages) const;
	void SetShaderResources(const Texture* const* aResourcesList, size_t aNumResources, unsigned int aStartSlot, PipelineStages aStages) const;
	void SetShaderSamplers(const Sampler* const* aSamplerList, size_t aNumOfSamplers, unsigned int aStartSlot, PipelineStages aStages) const;

	void UnbindShaderResource(unsigned int aSlot, PipelineStages aStages) const;

	void Draw(unsigned aNumVertices) const;
	void DrawSpecificVertices(unsigned aNumVertices, unsigned aFirstVertexIndex) const;
	void DrawIndexed(unsigned aIndexCount, unsigned aIndexOffset) const;
	void DrawIndexedInstanced(unsigned aIndexCount, unsigned aIndexOffset, unsigned aInstanceCount) const;

	void SetMarker(std::string_view aMarker) const;
	void BeginEvent(std::string_view aEvent) const;
	void EndEvent() const;

	void SetOverridePipelineState(const PipelineStateObject& aOverridePSO, PipelineStages aOverrideStages);
	void ChangeActiveOverrides(PipelineStages aOverrideStages);
	bool IsOverrideActive(PipelineStages aOverride) const;
	void ClearOverridePipelineState();

	void CopyRegion(const Texture& aSource, const Texture& aTarget, unsigned aX, unsigned aY, unsigned aWidth, unsigned aHeight);

	unsigned int GetPipelineSwitchCount() const;
	unsigned int GetDrawCallCount() const;

private:
	std::unique_ptr<PipelineStateObject> myOverridePipelineState;
	PipelineStages myCurrentOverrides;

	std::string myName;

	mutable unsigned int myPipelineSwitchCounter;
	mutable unsigned int myDrawCallCounter;

	Microsoft::WRL::ComPtr<struct ID3D11DeviceContext> myContext;
	Microsoft::WRL::ComPtr<struct ID3D11CommandList> myCommandList;
	Microsoft::WRL::ComPtr<struct ID3DUserDefinedAnnotation> myUDA;
};


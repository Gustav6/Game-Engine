#include "GraphicsEngine.pch.h"
#include "ConstantBufferManager.h"

#include "GraphicsEngine/RHI/RenderHardwareInterface.h"
#include "GraphicsEngine/RHI/GraphicsCommandList.h"

bool ConstantBufferManager::CreateConstantBuffer(RenderHardwareInterface& aRHI, ConstantBuffer aBufferID, std::string_view aName, size_t aBufferSize)
{
	Buffer buffer;

	if (!aRHI.CreateConstantBuffer(aName, aBufferSize, buffer))
	{
		return false;
	}

	myConstantBuffers.emplace(aBufferID, std::move(buffer));

	return true;
}

bool ConstantBufferManager::UpdateAndSetConstantBuffer(GraphicsCommandList& inoutCommandList, ConstantBuffer aBufferID, const void* aData, size_t aDataSize, unsigned aSlot, PipelineStages aStages) const
{
	if (!myConstantBuffers.contains(aBufferID))
	{
		GELOG(Warning, "Requested constant buffer update failed because this buffer does not exist!");
		return false;
	}

	const Buffer& buffer = myConstantBuffers.at(aBufferID);

	if (!inoutCommandList.UpdateConstantBuffer(buffer, aData, aDataSize))
	{
		return false;
	}

	inoutCommandList.SetConstantBuffer(&buffer, aSlot, aStages);

	return true;
}

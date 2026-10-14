#include "GraphicsEngine.pch.h"
#include "ConstantBufferManager.h"

#include "ConstantBuffers/FrameBuffer.h"
#include "ConstantBuffers/ObjectBuffer.h"
#include "ConstantBuffers/AnimationBuffer.h"
#include "ConstantBuffers/LightBuffer.h"
#include "ConstantBuffers/CascadeBuffer.h"

#include "Core/RenderEnums.h"

#include "Materials/Material.h"

#include "GraphicsEngine/RHI/RenderHardwareInterface.h"
#include "GraphicsEngine/RHI/GraphicsCommandList.h"

bool ConstantBufferManager::Initialize(RenderHardwareInterface& aRHI)
{
	if (!CreateConstantBuffer<FrameBuffer>(aRHI, ConstantBuffer::FrameBuffer, "FrameBuffer"))
	{
		spdlog::critical("Failed to create Frame Buffer!");
		return false;
	}
	if (!CreateConstantBuffer<ObjectBuffer>(aRHI, ConstantBuffer::ObjectBuffer, "ObjectBuffer"))
	{
		spdlog::critical("Failed to create Object Buffer!");
		return false;
	}
	if (!CreateConstantBuffer<AnimationBuffer>(aRHI, ConstantBuffer::AnimationBuffer, "AnimationBuffer"))
	{
		spdlog::critical("Failed to create Animation Buffer!");
		return false;
	}
	if (!CreateConstantBuffer<LightBuffer>(aRHI, ConstantBuffer::LightBuffer, "LightBuffer"))
	{
		spdlog::critical("Failed to create Light Buffer!");
		return false;
	}
	if (!CreateConstantBuffer<CascadeBuffer>(aRHI, ConstantBuffer::CascadeBuffer, "CascadeBuffer"))
	{
		spdlog::critical("Failed to create Cascade Buffer!");
		return false;
	}
	if (!CreateConstantBuffer(aRHI, ConstantBuffer::MaterialBuffer, "MaterialBuffer", Material::MATERIAL_BUFFER_SIZE))
	{
		spdlog::critical("Failed to create Material Buffer!");
		return false;
	}

	return true;
}

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
		spdlog::warn("Requested constant buffer update failed because this buffer does not exist!");
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

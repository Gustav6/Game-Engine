#include "Graphics/GraphicsEngine.pch.h"
#include "BufferManager.h"

#include "Graphics/Buffers/FrameBuffer.h"
#include "Graphics/Buffers/ObjectBuffer.h"
#include "Graphics/Buffers/AnimationBuffer.h"
#include "Graphics/Buffers/LightBuffer.h"
#include "Graphics/Buffers/CascadeBuffer.h"

#include "Graphics/Core/RenderEnums.h"

#include "Graphics/Materials/Material.h"

#include "Graphics/RHI/RenderHardwareInterface.h"
#include "Graphics/RHI/GraphicsCommandList.h"

bool BufferManager::Initialize(RenderHardwareInterface& aRHI)
{
	if (!CreateConstantBuffer<FrameBuffer>(aRHI, ConstantBuffer::FrameBuffer, "FrameBuffer"))
	{
		SPDLOG_CRITICAL("Failed to create Frame Buffer!");
		return false;
	}
	if (!CreateConstantBuffer<ObjectBuffer>(aRHI, ConstantBuffer::ObjectBuffer, "ObjectBuffer"))
	{
		SPDLOG_CRITICAL("Failed to create Object Buffer!");
		return false;
	}
	if (!CreateConstantBuffer<AnimationBuffer>(aRHI, ConstantBuffer::AnimationBuffer, "AnimationBuffer"))
	{
		SPDLOG_CRITICAL("Failed to create Animation Buffer!");
		return false;
	}
	if (!CreateConstantBuffer<LightBuffer>(aRHI, ConstantBuffer::LightBuffer, "LightBuffer"))
	{
		SPDLOG_CRITICAL("Failed to create Light Buffer!");
		return false;
	}
	if (!CreateConstantBuffer<CascadeBuffer>(aRHI, ConstantBuffer::CascadeBuffer, "CascadeBuffer"))
	{
		SPDLOG_CRITICAL("Failed to create Cascade Buffer!");
		return false;
	}
	if (!CreateConstantBuffer(aRHI, ConstantBuffer::MaterialBuffer, "MaterialBuffer", Material::MATERIAL_BUFFER_SIZE))
	{
		SPDLOG_CRITICAL("Failed to create Material Buffer!");
		return false;
	}

	return true;
}

bool BufferManager::CreateConstantBuffer(RenderHardwareInterface& aRHI, ConstantBuffer aBufferID, std::string_view aName, size_t aBufferSize)
{
	Buffer buffer;

	if (!aRHI.CreateConstantBuffer(aName, aBufferSize, buffer))
	{
		return false;
	}

	myBuffers.emplace(aBufferID, std::move(buffer));

	return true;
}

bool BufferManager::UpdateAndSetConstantBuffer(GraphicsCommandList& inoutCommandList, ConstantBuffer aBufferID, const void* aData, size_t aDataSize, unsigned aSlot, PipelineStages aStages) const
{
	if (!myBuffers.contains(aBufferID))
	{
		SPDLOG_WARN("Requested constant buffer update failed because this buffer does not exist!");
		return false;
	}

	const Buffer& buffer = myBuffers.at(aBufferID);

	if (!inoutCommandList.UpdateConstantBuffer(buffer, aData, aDataSize))
	{
		return false;
	}

	inoutCommandList.SetConstantBuffer(&buffer, aSlot, aStages);

	return true;
}

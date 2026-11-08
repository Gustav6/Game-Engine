#include "Graphics/GraphicsEngine.pch.h"
#include "ConstantBufferManager.h"

#include "Graphics/ConstantBuffers/FrameBuffer.h"
#include "Graphics/ConstantBuffers/ObjectBuffer.h"
#include "Graphics/ConstantBuffers/AnimationBuffer.h"
#include "Graphics/ConstantBuffers/LightBuffer.h"
#include "Graphics/ConstantBuffers/CascadeBuffer.h"

#include "Graphics/Core/RenderEnums.h"

#include "Graphics/Materials/Material.h"

#include "Graphics/RHI/RenderHardwareInterface.h"
#include "Graphics/RHI/GraphicsCommandList.h"

bool ConstantBufferManager::Initialize(RenderHardwareInterface& aRHI)
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
		SPDLOG_WARN("Requested constant buffer update failed because this buffer does not exist!");
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

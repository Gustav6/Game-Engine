#pragma once

#include <unordered_map>
#include <string_view>
#include <cstdint>

#include "Graphics/Objects/GPU/Buffer.h"

class RenderHardwareInterface;
class GraphicsCommandList;

enum class ConstantBuffer : uint8_t;

class ConstantBufferManager
{
public:
	bool Initialize(RenderHardwareInterface& aRHI);

    bool CreateConstantBuffer(RenderHardwareInterface& aRHI, ConstantBuffer aBufferID, std::string_view aName, size_t aBufferSize);

    template<class T>
    bool CreateConstantBuffer(RenderHardwareInterface& aRHI, ConstantBuffer aBufferID, std::string_view aName)
    {
        return CreateConstantBuffer(aRHI, aBufferID, aName, sizeof(T));
    }

    bool UpdateAndSetConstantBuffer(GraphicsCommandList& inoutCommandList, ConstantBuffer aBufferID, const void* aData, size_t aDataSize, unsigned aSlot, PipelineStages aStages) const;

    template<class T>
    bool UpdateAndSetConstantBuffer(GraphicsCommandList& inoutCommandList, ConstantBuffer aBufferID, const T& aData, unsigned aSlot, PipelineStages aStages) const
    {
        return UpdateAndSetConstantBuffer(inoutCommandList, aBufferID, &aData, sizeof(T), aSlot, aStages);
    }

private:
    std::unordered_map<ConstantBuffer, Buffer> myConstantBuffers;
};
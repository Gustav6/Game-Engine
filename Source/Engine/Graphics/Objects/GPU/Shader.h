#pragma once

#include <wrl/client.h>
#include <d3dcommon.h>

#include "Graphics/RHI/RHIStructs.h"

struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;
typedef unsigned char uint8_t;

class Shader
{
	friend class RenderHardwareInterface;

public:
	Shader();
	~Shader();

	ShaderType GetShaderType() const { return myType; }
	const uint8_t* GetDataPtr() const { return static_cast<uint8_t*>(myBlob->GetBufferPointer()); }
	size_t GetDataSize() const { return myBlob->GetBufferSize(); }

private:
	Microsoft::WRL::ComPtr<ID3DBlob> myBlob;
	ShaderType myType = ShaderType::Unknown;
};
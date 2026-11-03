#pragma once

#include <wrl/client.h>
#include <string>

#include "Graphics/RHI/RHIStructs.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

class Buffer
{
	friend class RenderHardwareInterface;
	friend class GraphicsCommandList;

public:
	Buffer();
	~Buffer();

	bool IsValid() const
	{
		return myBuffer && myType != BufferType::Unknown;
	}

private:
	std::string myName;
	Microsoft::WRL::ComPtr<ID3D11Buffer> myBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mySRV;
	size_t mySize;
	unsigned myStride;
	BufferType myType;
};
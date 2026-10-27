#pragma once

#include <wrl.h>
#include <d3d11.h>
#include "Graphics/RHI/RHIStructs.h"


class Texture
{
	friend class RenderHardwareInterface;
	friend class GraphicsCommandList;

public:
	Texture();
	~Texture();

	std::string_view GetName() const { return myName; }
	const Viewport& GetViewport() const { return myViewport; }

private:
	std::string myName;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> myRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> myDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mySRV;

	Viewport myViewport;
};
#pragma once

#include <wrl.h>
#include "Graphics/RHI/RHIStructs.h"

struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct ID3D11Resource;

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
	Microsoft::WRL::ComPtr<ID3D11Resource> myRHITexture;

	Viewport myViewport;
};
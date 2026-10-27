#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;

class RenderOverlay
{
public:
	virtual ~RenderOverlay() = default;

	virtual void OnAttach(ID3D11Device* aDevice, ID3D11DeviceContext* aContext) = 0;
	virtual void OnDetach() = 0;

	virtual void BeginFrame() = 0;
	virtual void Render() = 0;
};

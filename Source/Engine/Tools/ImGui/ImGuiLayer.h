#pragma once

#include "Graphics/Core/RenderOverlay.h"

class ImGuiLayer : public RenderOverlay
{
public:
	ImGuiLayer(HWND aHWND);

    void OnAttach(ID3D11Device* aDevice, ID3D11DeviceContext* aContext) override;
    void OnDetach() override;

    void BeginFrame() override;
    void Render() override;

    static bool HandleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HWND myWindowHandle;
};
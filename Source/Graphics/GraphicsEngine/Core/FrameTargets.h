#pragma once

class Texture;

struct FrameTargets
{
    Texture* RenderTarget = nullptr;
    Texture* DepthBuffer = nullptr;
};
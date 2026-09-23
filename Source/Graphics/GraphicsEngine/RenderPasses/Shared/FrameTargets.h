#pragma once

class Texture;
class GBuffer;

struct FrameTargets
{
    Texture* BackBuffer = nullptr;
    Texture* DepthBuffer = nullptr;
};
#pragma once

class Texture;

struct FrameTargets
{
    Texture* BackBuffer = nullptr;
    Texture* DepthBuffer = nullptr;
};
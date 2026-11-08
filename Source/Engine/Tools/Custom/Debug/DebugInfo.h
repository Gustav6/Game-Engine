#pragma once

struct DebugInfo
{
    float FPS = 0.0f;
    float FrameTime = 0.0f;

    unsigned int DrawCallCount = 0;
    unsigned int PipelineSwitches = 0;

    unsigned int VisibleObjects = 0;
    unsigned int CulledObjects = 0;
};
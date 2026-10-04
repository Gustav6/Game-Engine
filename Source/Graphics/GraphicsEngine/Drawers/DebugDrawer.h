#pragma once

#include <vector>

#include <CommonUtilities/Math/Matrix4x4.hpp>

class Mesh;

struct DebugDrawCall
{
    const Mesh* Mesh = nullptr;
    CommonUtilities::Matrix4x4f Transform;
};

class DebugDrawer
{
    friend class GraphicsEngine;

public:
    static DebugDrawer& Get();

    void Submit(const DebugDrawCall& aDrawCall);

private:
    std::vector<DebugDrawCall> myDebugDrawCalls;
};
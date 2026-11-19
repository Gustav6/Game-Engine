#include "Tools/Tools.pch.h"
#include "ToolRegistry.h"

#include <Tools/Custom/Debug/DebugTool.h>

#include <Tools/Editor/Hierarchy/SceneHierarchy.h>
#include <Tools/Editor/Viewport/SceneViewport.h>
#include <Tools/Editor/AssetBrowser/AssetBrowser.h>

ToolRegistry& ToolRegistry::Get()
{
    static ToolRegistry myInstance;
    return myInstance;
}

ToolRegistry::ToolRegistry() = default;
ToolRegistry::~ToolRegistry() = default;

bool ToolRegistry::Initialize()
{
    RegisterTool<DebugTool>();
    RegisterTool<SceneHierarchy>();
    RegisterTool<SceneViewport>();
    RegisterTool<AssetBrowser>();

    return true;
}
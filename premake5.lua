-- Name of the VS Solution.
workspace "GameProject"
    -- What configurations we will have.
    configurations { "Debug", "Release", "Retail" }
    platforms { "x64" }
    startproject "Game"
    language "C++"
    cppdialect "C++20"
    systemversion "latest"
    toolset "v145"
    
    defines { "SPDLOG_COMPILED_LIB", "FMT_UNICODE=0" }
    externalwarnings "Off"

filter { "platforms:x64" }
    system "Windows"
    architecture "x86_64"
    buildoptions "/sdl"

-- For Debug configuration, we want to set the _DEBUG #define flag and turn on symbol generation.
filter "configurations:Debug"
    multiprocessorcompile "On"
    defines { "_DEBUG", "IMGUI_ENABLE", "TRACY_ENABLE", "TRACY_ON_DEMAND", "ENABLE_FILE_WATCHER", "ENABLE_LOGGER" }
    symbols "On"

-- Generate symbols in release mode so we can debug the release build if needed.
filter "configurations:Release"
    multiprocessorcompile "On"
    defines { "_RELEASE", "IMGUI_ENABLE", "ENABLE_FILE_WATCHER", "ENABLE_LOGGER" }
    fatalwarnings { "All" }
    optimize "On"
    symbols "On"

-- Settings to use for retail build used for shipping the game.
filter "configurations:Retail"
    multiprocessorcompile "On"
    fatalwarnings { "All" }
    defines { "_RETAIL" }
    optimize "On"

filter {} 

group "Utilities"
    include "Source/Utilities/CommonUtilities/premake5.lua"
    include "Source/Utilities/Logger/premake5.lua"
    include "Source/Utilities/FileWatcher/premake5.lua"

group "GameFramework"
    include "Source/GameFramework/premake5.lua"

group "GraphicsEngine"
    include "Source/Graphics/GraphicsEngine/premake5.lua"

group "ThirdParty"
    include "Source/ThirdParty/ImGui/premake5.lua"
    include "Source/ThirdParty/Json/premake5.lua"

group "Application"
    include "Source/Application/Game/premake5.lua"
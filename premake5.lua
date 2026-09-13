-- Name of the VS Solution.
workspace "GameProject"
    -- What configuration we will have.
    configurations { "Debug", "Release", "Retail" }
    -- Platforms we will support, always use x64 for 64-bit builds.
    platforms { "x64" }
    toolset "v145"
    startproject "Game"

filter { "platforms:x64" }
    system "Windows"
    architecture "x86_64"
    buildoptions "/sdl"

-- For Debug configuration, we want to set the _DEBUG #define flag and turn on symbol generation.
filter "configurations:Debug"
    multiprocessorcompile "On"
    warnings "Extra"
    defines { "_DEBUG", "TRACY_ENABLE" }
    symbols "On"

-- Generate symbols in release mode so we can debug the release build if needed.
filter "configurations:Release"
    multiprocessorcompile "On"
    warnings "Extra"
    fatalwarnings { "All" }
    optimize "On"
    symbols "On"

-- Settings to use for retail build used for shipping the game.
filter "configurations:Retail"
    multiprocessorcompile "On"
    warnings "Extra"
    fatalwarnings { "All" }
    defines { "_RETAIL" }
    optimize "On"
    enableunitybuild "On"

filter {} 

group "Dependencies"
    include "Source/Graphics/GraphicsEngine/premake5.lua"
    include "Source/Utilities/CommonUtilities/premake5.lua"
    include "Source/Utilities/Logger/premake5.lua"

group "Game"
    include "Source/Application/Game/premake5.lua"
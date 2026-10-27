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
    defines { "_DEBUG", "TRACY_ENABLE" }
    --defines { "_DEBUG" }
    symbols "On"

-- Generate symbols in release mode so we can debug the release build if needed.
filter "configurations:Release"
    multiprocessorcompile "On"
    defines { "_RELEASE" }
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

group "Utilities/Wrappers"
    include "Source/Utilities/Wrappers/Logger/premake5.lua"
    include "Source/Utilities/Wrappers/FileWatcher/premake5.lua"
    include "Source/Utilities/Wrappers/Json/premake5.lua"
    
group "Engine"
    include "Source/Engine/Core/premake5.lua"
    include "Source/Engine/Graphics/premake5.lua"
    include "Source/Engine/Editor/premake5.lua"

group "Application"
    include "Source/Application/premake5.lua"
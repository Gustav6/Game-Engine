project "Editor"
    kind "StaticLib"

    pchheader "Editor.pch.h"
    pchsource "Editor.pch.cpp"

    warnings "Extra"
    fatalwarnings { "All" }

    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{prj.location}",
        "%{wks.location}/Source/Utilities",
        "%{wks.location}/Source/Engine"
    }

    externalincludedirs {
        "%{wks.location}/ThirdParty/Source/imgui",
        "%{wks.location}/ThirdParty/Source"
    }

    files { 
        "%{prj.location}/**.h", 
        "%{prj.location}/**.cpp", 
        "%{prj.location}/**.hpp",
        
        -- Third Party
        "%{wks.location}/ThirdParty/Source/imgui/imgui.cpp",
        "%{wks.location}/ThirdParty/Source/imgui/imgui_draw.cpp",
        "%{wks.location}/ThirdParty/Source/imgui/imgui_tables.cpp",
        "%{wks.location}/ThirdParty/Source/imgui/imgui_widgets.cpp",
        "%{wks.location}/ThirdParty/Source/imgui/backends/imgui_impl_win32.cpp",
        "%{wks.location}/ThirdParty/Source/imgui/backends/imgui_impl_dx11.cpp",
        "%{wks.location}/ThirdParty/Source/imgui/imgui_demo.cpp",
    }

    filter "files:**/imgui/**.cpp"
        enablepch "Off"
        warnings "Off"
        fatalwarnings { }
    filter {}

    links {
        "CommonUtilities",
        "Logger",
        "Core",
        "Graphics",
        "Json"
    }
    
    filter "configurations:Retail"
        kind "None"

    filter {}
project "ImGui"
    kind "StaticLib"

    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir    ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{prj.location}",
        "%{wks.location}/ThirdParty/Source",
        "%{wks.location}/Source/Graphics",
        "%{wks.location}/ThirdParty/Source/imgui",
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

project "Framework"
    kind "StaticLib"

    warnings "Extra"
    fatalwarnings { "All" }

    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{prj.location}",
        "%{wks.location}/Source/Utilities",
        "%{wks.location}/Source/Graphics",
        "%{wks.location}/ThirdParty/Source"
    }

    files {
        "%{prj.location}/**.h",
        "%{prj.location}/**.cpp",
        "%{prj.location}/**.hpp",
        
        -- Third Party
        "%{wks.location}/ThirdParty/Source/tracy/TracyClient.cpp" 
    }

    links {
        "CommonUtilities",
        "Logger",
        "GraphicsEngine"
    }
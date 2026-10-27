project "Core"
    kind "StaticLib"

    pchheader "Core.pch.h"
    pchsource "Core.pch.cpp"

    warnings "Extra"
    fatalwarnings { "All" }

    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{wks.location}/Source/Utilities",
        "%{wks.location}/Source/Engine"
    }

    externalincludedirs {
        "%{wks.location}/ThirdParty/Source",
    }

    files {
        "%{prj.location}/**.h",
        "%{prj.location}/**.cpp",
        "%{prj.location}/**.hpp"
    }
    
    links {
        "CommonUtilities",
        "Logger",
        "Graphics",
        "Json"
    }
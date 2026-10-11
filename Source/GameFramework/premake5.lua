project "Framework"
    kind "StaticLib"

    pchheader "GameFramework.pch.h"
    pchsource "GameFramework.pch.cpp"

    warnings "Extra"
    fatalwarnings { "All" }

    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{prj.location}",
        "%{wks.location}/Source/Utilities",
        "%{wks.location}/Source/Graphics",
        "%{wks.location}/Source/ThirdParty",
        "%{wks.location}/ThirdParty/Source"
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
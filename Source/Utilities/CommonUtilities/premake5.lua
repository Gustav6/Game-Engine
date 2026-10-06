project "CommonUtilities"
    kind "StaticLib"

    pchheader "CommonUtilities.pch.h"
    pchsource "CommonUtilities.pch.cpp"

    warnings "Extra"
    fatalwarnings { "All" }

    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{prj.location}",
        "%{wks.location}/Source/Utilities",
        "%{wks.location}/ThirdParty/Source"
    }
    
    files { 
        "%{prj.location}/**.h", 
        "%{prj.location}/**.cpp", 
        "%{prj.location}/**.hpp"
    }

    links {
        "Logger"
    }

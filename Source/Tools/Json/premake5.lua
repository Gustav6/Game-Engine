project "Json"
    kind "StaticLib"

    enablepch "Off"

    warnings "Extra"
    fatalwarnings { "All" }

    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{prj.location}",
        "%{wks.location}/Source/Utilities",
        "%{wks.location}/ThirdParty/Source/yyjson",
    }
    
    files { 
        "%{prj.location}/**.h", 
        "%{prj.location}/**.cpp", 
        "%{prj.location}/**.hpp", 

        -- Third Party
        "%{wks.location}/ThirdParty/Source/yyjson/yyjson.c"
    }

    filter "files:**/yyjson.c"
        enablepch "Off"
        warnings "Off"

    filter {}
    
    links {
        "Logger"
    }


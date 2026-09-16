project "Framework"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    -- warnings "Extra"
    -- fatalwarnings { "All" }

    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir    ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    vsprops {
        DisableFastUpToDateCheck = "true"
    }

    externalincludedirs { "%{prj.location}", "$(VC_IncludePath)", "$(WindowsSDK_IncludePath)" }

    includedirs {
        "%{wks.location}/Source/Utilities",
        "%{wks.location}/Source/Graphics",
        "%{wks.location}/ThirdParty/Source",
        
        "%{wks.location}/ThirdParty/Source/yyjson/yyjson.c", 
        "%{wks.location}/ThirdParty/Source/tracy/TracyClient.cpp" 
    }

    files {
        "%{prj.location}/**.h",
        "%{prj.location}/**.cpp",
        "%{prj.location}/**.hpp",
        "%{prj.location}/**.inl",
        "%{prj.location}/**.cc",
        
        "%{wks.location}/ThirdParty/Source/yyjson/yyjson.c", 
        "%{wks.location}/ThirdParty/Source/tracy/TracyClient.cpp" 
    }

    links {
        "CommonUtilities",
        "Logger",
        "GraphicsEngine"
    }
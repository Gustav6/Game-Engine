project "CommonUtilities"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir    ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    vsprops {
        DisableFastUpToDateCheck = "true"
    }
    
    externalincludedirs { "%{prj.location}", "$(VC_IncludePath)", "$(WindowsSDK_IncludePath)" }

    files { 
        "%{prj.location}/**.h", 
        "%{prj.location}/**.cpp", 
        "%{prj.location}/**.hpp", 
        "%{prj.location}/**.inl", 
        "%{prj.location}/**.cc"
    }

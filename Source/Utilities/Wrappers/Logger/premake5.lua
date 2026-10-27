project "Logger"
    kind "StaticLib"

    defines { "SPDLOG_COMPILED_LIB" }
    
    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir    ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{prj.location}",
    }

    externalincludedirs {
        "%{wks.location}/ThirdParty/Source"
    }

    files { 
        "%{prj.location}/**.h", 
        "%{prj.location}/**.cpp", 
        "%{prj.location}/**.hpp",

        -- Third Party
        "%{wks.location}/ThirdParty/Source/spdlog/src/spdlog.cpp",
        "%{wks.location}/ThirdParty/Source/spdlog/src/stdout_sinks.cpp",
        "%{wks.location}/ThirdParty/Source/spdlog/src/color_sinks.cpp",
        "%{wks.location}/ThirdParty/Source/spdlog/src/file_sinks.cpp",
        "%{wks.location}/ThirdParty/Source/spdlog/src/async.cpp",
        "%{wks.location}/ThirdParty/Source/spdlog/src/cfg.cpp",
        "%{wks.location}/ThirdParty/Source/spdlog/src/bundled_fmtlib_format.cpp"
    }


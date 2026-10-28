function conf_excludes()
    local base = "%{wks.location}/ThirdParty/Source/efsw/"

    if os.istarget("windows") then
        removefiles {
            base .. "WatcherKqueue.cpp",
            base .. "WatcherFSEvents.cpp",
            base .. "WatcherInotify.cpp",
            base .. "FileWatcherKqueue.cpp",
            base .. "FileWatcherInotify.cpp",
            base .. "FileWatcherFSEvents.cpp",
        }
    end
end

function conf_platform_files()
    local base = "%{wks.location}/ThirdParty/Source/efsw/platform/"

    if os.istarget("windows") then
        files { base .. "win/*.cpp" }
    else
        files { base .. "posix/*.cpp" }
    end
end

project "FileWatcher"
    kind "StaticLib"

    enablepch "Off"

    warnings "Extra"
    fatalwarnings { "All" }

    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{prj.location}"
    }
    
    externalincludedirs {
        "%{wks.location}/ThirdParty/Source",
    }

    files { 
        "%{prj.location}/**.h", 
        "%{prj.location}/**.cpp", 
        "%{prj.location}/**.hpp", 

        -- Third Party
        "%{wks.location}/ThirdParty/Source/efsw/*.cpp"
    }

    conf_excludes()
    conf_platform_files()

    filter "files:**/efsw/**.cpp"
        enablepch "Off"
        warnings "Off"
        fatalwarnings { }
    filter {}

    filter "configurations:Debug"
        defines { "DEBUG" }

    filter "configurations:Release"
        defines { "NDEBUG" }

    filter "configurations:Retail"
        kind "None"

    filter {}


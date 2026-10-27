function conf_excludes()
    local base = "%{wks.location}/ThirdParty/Source/efsw/src/efsw/"

    if os.istarget("windows") then
        removefiles {
            base .. "WatcherKqueue.cpp", base .. "WatcherFSEvents.cpp", base .. "WatcherInotify.cpp",
            base .. "FileWatcherKqueue.cpp", base .. "FileWatcherInotify.cpp", base .. "FileWatcherFSEvents.cpp",
        }
    elseif os.istarget("linux") then
        removefiles {
            base .. "WatcherKqueue.cpp", base .. "WatcherFSEvents.cpp", base .. "WatcherWin32.cpp",
            base .. "FileWatcherKqueue.cpp", base .. "FileWatcherWin32.cpp", base .. "FileWatcherFSEvents.cpp",
        }
    elseif os.istarget("macosx") then
        removefiles {
            base .. "WatcherInotify.cpp", base .. "WatcherWin32.cpp",
            base .. "FileWatcherInotify.cpp", base .. "FileWatcherWin32.cpp",
        }
    elseif os.istarget("bsd") then
        removefiles {
            base .. "WatcherInotify.cpp", base .. "WatcherWin32.cpp", base .. "WatcherFSEvents.cpp",
            base .. "FileWatcherInotify.cpp", base .. "FileWatcherWin32.cpp", base .. "FileWatcherFSEvents.cpp",
        }
    end
end

function conf_platform_files()
    local base = "%{wks.location}/ThirdParty/Source/efsw/src/efsw/platform/"
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
        "%{wks.location}/ThirdParty/Source/efsw/include",
        "%{wks.location}/ThirdParty/Source/efsw/src"
    }

    files { 
        "%{prj.location}/**.h", 
        "%{prj.location}/**.cpp", 
        "%{prj.location}/**.hpp", 

        -- Third Party
        "%{wks.location}/ThirdParty/Source/efsw/src/efsw/*.cpp"
    }

    conf_excludes()
    conf_platform_files()

    filter "configurations:Debug"
        defines { "DEBUG" }

    filter "configurations:Release"
        defines { "NDEBUG" }

    filter "configurations:Retail"
        kind "None"

    filter {}


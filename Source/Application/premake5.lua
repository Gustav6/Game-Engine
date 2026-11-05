project "Game"
    kind "WindowedApp"

    warnings "Extra"
    fatalwarnings { "All" }

    targetdir ("%{wks.location}/Bin/%{cfg.buildcfg}/")
    objdir ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{prj.location}",
        "%{wks.location}/Source/Utilities",
        "%{wks.location}/Source/Engine"
    }

    externalincludedirs {
        "%{wks.location}/ThirdParty/Source"
    }

    files {
        "%{prj.location}/**.h",
        "%{prj.location}/**.cpp",
        "%{prj.location}/**.hpp",

        "%{prj.location}/**.hlsl",
        "%{prj.location}/**.hlsli",
        
        -- Third Party
        "%{wks.location}/ThirdParty/Source/tracy/TracyClient.cpp" 
    }

    links {
        "Logger",
        "Graphics",
        "Core",
        "CommonUtilities",
        "FileWatcher",
        "Json",

        -- Third Party
        "TGAFbx",
        "fmod_vc",
        "fmodstudio_vc"
    }

    libdirs {
        "%{wks.location}/ThirdParty/Lib/%{cfg.buildcfg}/"
    }

    dependson {
        "CommonUtilities",
        "Logger",
        "GraphicsEngine",
        "FileWatcher",
        "Core",
        "Json"
    }

    filter "configurations:Debug"
        links { "Tools" }
        dependson { "Tools" }

    filter "configurations:Release"
        links { "Tools" }
        dependson { "Tools" }

    postbuildmessage "Copying Materials and DLLs"

    filter "files:**.hlsl or files:**.hlsli"
        buildmessage "Copying %{file.relpath}"
        buildcommands {
            'if not exist "$(SolutionDir)Shaders" mkdir "$(SolutionDir)Shaders"',
            'copy /Y "%{file.abspath}" "$(SolutionDir)Shaders\\%{file.name}"'
        }
        buildoutputs { "$(SolutionDir)Shaders\\%{file.name}" }
        
    filter {}

    postbuildcommands {
        'IF EXIST "$(SolutionDir)ThirdParty\\Dll" ( xcopy /E /I /R /Y /D "$(SolutionDir)ThirdParty\\Dll" "$(OutDir)" )'
    }

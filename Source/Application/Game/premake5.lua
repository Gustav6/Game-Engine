project "Game"
    kind "WindowedApp"

    warnings "Extra"
    fatalwarnings { "All" }

    targetdir ("%{wks.location}/Bin/%{cfg.buildcfg}/")
    objdir ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{prj.location}",
        "%{wks.location}/Source/Utilities",
        "%{wks.location}/Source/Graphics",
        "%{wks.location}/Source/GameFramework",
        "%{wks.location}/Source/Tools",
        "%{wks.location}/ThirdParty/Source"
    }

    files {
        "%{prj.location}/**.h",
        "%{prj.location}/**.cpp",
        "%{prj.location}/**.hpp",
        "%{prj.location}/**.inl",
        
        -- Third Party
        "%{wks.location}/ThirdParty/Source/tracy/TracyClient.cpp" 
    }

    links {
        "Logger",
        "Graphics",
        "Framework",
        "CommonUtilities",
        "ImGui",

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
        "Framework",
        "ImGui" -- Move to only depnd on ImGui if we want to use it in the game project. (Debug and Release builds)
    }

    postbuildmessage "Copying Materials and DLLs"
    
    postbuildcommands {
        'set "CONTENTROOT=$(SolutionDir)Content"',
        'set "SHADERSRC=$(ProjectDir)Materials"',
        'set "SHADERDEST=%CONTENTROOT%\\Shaders"',
        '',
        'IF NOT EXIST "%CONTENTROOT%" ( mkdir "%CONTENTROOT%" )',
        'IF NOT EXIST "%SHADERDEST%" ( mkdir "%SHADERDEST%" )',
        'xcopy /E /I /R /Y /D "%SHADERSRC%" "%SHADERDEST%"',
        '',
        'IF EXIST "$(SolutionDir)ThirdParty\\Dll" ( xcopy /E /I /R /Y /D "$(SolutionDir)ThirdParty\\Dll" "$(OutDir)" )'
    }

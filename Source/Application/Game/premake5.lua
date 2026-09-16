project "Game"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"

    fatalwarnings { "All" }

    filter "configurations:Release"
        warnings "Extra"

    filter "configurations:Retail"
        warnings "Extra"

    filter {}

    targetdir ("%{wks.location}/Bin/%{cfg.buildcfg}/")
    objdir    ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    vsprops {
        DisableFastUpToDateCheck = "true"
    }
    
    externalincludedirs { "%{prj.location}", "$(VC_IncludePath)", "$(WindowsSDK_IncludePath)" }

    includedirs {
        "%{wks.location}/Source/Utilities",
        "%{wks.location}/Source/Graphics",
        "%{wks.location}/Source/GameFramework",
        "%{wks.location}/ThirdParty/Source",
        "%{wks.location}/Source/Graphics/GraphicsEngine/Shaders/Material"
    }

    files {
        "%{prj.location}/**.h",
        "%{prj.location}/**.cpp",
        "%{prj.location}/**.hpp",
        "%{prj.location}/**.inl",
        "%{prj.location}/**.cc",
        "%{prj.location}/**.hlsli"
    }

    libdirs {
        "%{wks.location}/ThirdParty/Lib/%{cfg.buildcfg}/"
    }

    links {
        "Logger",
        "GraphicsEngine",
        "Framework",
        "CommonUtilities",
        "d3d11",
        "dxguid",
        "dxgi",
        "d3dcompiler",
        "TGAFbx",
        "fmod_vc",
        "fmodstudio_vc"
    }

    dependson {
        "CommonUtilities",
        "Logger",
        "GraphicsEngine",
        "Framework"
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

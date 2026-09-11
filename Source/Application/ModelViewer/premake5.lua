project "Game"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"

    -- Fix: Use Premake tokens so Visual Studio can resolve paths natively for tracking
    targetdir ("%{wks.location}/Bin/%{cfg.buildcfg}/")
    objdir    ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    externalincludedirs { "%{prj.location}", "$(VC_IncludePath)", "$(WindowsSDK_IncludePath)" }

    includedirs {
        "%{wks.location}/Source/Utilities",
        "%{wks.location}/Source/Graphics",
        "%{wks.location}/ThirdParty/Source",
        "%{wks.location}/Source/Graphics/GraphicsEngine/Shaders/Material"
    }

    files {
        "%{prj.location}/**.h",
        "%{prj.location}/**.cpp",
        "%{prj.location}/**.hpp",
        "%{prj.location}/**.inl",
        "%{prj.location}/**.cc",
        "%{prj.location}/**.hlsli",
        
        "%{wks.location}/ThirdParty/Source/yyjson/yyjson.c", 
        "%{wks.location}/ThirdParty/Source/tracy/TracyClient.cpp" 
    }

    libdirs {
        "%{wks.location}/ThirdParty/Lib/%{cfg.buildcfg}/"
    }

    links {
        "Logger",
        "GraphicsEngine",
        "CommonUtilities",
        "d3d11",
        "dxguid",
        "dxgi",
        "d3dcompiler",
        "TGAFbx"
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

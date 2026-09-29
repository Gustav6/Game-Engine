project "Graphics"
    kind "StaticLib"

    pchheader "GraphicsEngine.pch.h"
    pchsource "GraphicsEngine.pch.cpp"

    warnings "Extra"
    fatalwarnings { "All" }
    
    targetdir ("%{wks.location}/Lib/%{cfg.buildcfg}/")
    objdir ("%{wks.location}/Intermediate/%{prj.name}/%{cfg.buildcfg}/")

    includedirs {
        "%{prj.location}",
        "%{wks.location}/Source/Graphics/",
        "%{wks.location}/Source/Utilities/",
        "%{wks.location}/Intermediate/GraphicsEngine/%{cfg.buildcfg}/PrecompiledShaders/"
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
    
    filter "files:**/TracyClient.cpp"
        enablepch "Off"
        warnings "Off"

    filter {}

    links {
        "CommonUtilities",
        "Logger"
    }

    shadermodel "5.0"
    shaderentry "main"

    filter "files:**.hlsl"
        shaderobjectfileoutput ""
        shaderheaderfileoutput "%{wks.location}\\Intermediate\\GraphicsEngine\\%{cfg.buildcfg}\\PrecompiledShaders\\%%(Filename).h"
        shadervariablename "INTERNAL_%%(Filename)_ByteCode"

        vsprops {
            ParallelCompilation = "true"
        }

    filter "files:**_VS.hlsl"
        shadertype "Vertex"

    filter "files:**_PS.hlsl"
        shadertype "Pixel"

    filter "files:**_GS.hlsl"
        shadertype "Geometry"

    filter "files:**_CS.hlsl"
        shadertype "Compute"
        
    filter {}
    
    postbuildmessage "Copying Shaders to Content Dir"
    postbuildcommands {
        'set "CONTENTROOT=$(SolutionDir)Content"',
        'set "SHADERSRC=$(ProjectDir)Shaders"',
        'set "SHADERDEST=%CONTENTROOT%\\Shaders"',
        '',
        'IF NOT EXIST "%CONTENTROOT%" ( mkdir "%CONTENTROOT%" )',
        'IF NOT EXIST "%SHADERDEST%" ( mkdir "%SHADERDEST%" )',
        'xcopy /E /I /R /Y /D "%SHADERSRC%" "%SHADERDEST%"'
    }

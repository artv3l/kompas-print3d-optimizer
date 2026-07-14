project "kompas-print3d-optimizer"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetextension (".rtw")
    characterset ("Unicode")
    mfc "On"

    pchheader "stdafx.h"
	pchsource "src/stdafx.cpp"

    files { "**.hpp", "**.h", "**.cpp", "**.rc" }

    forceincludes  { "stdafx.h" }

    includedirs {
        "src",
        "%{wks.location}/oglwrap/include",
        "%{wks.location}/kapiwrap/include",
        "%{wks.location}/generic/include",
        "%{wks.location}/core/include",

        "%{localDependencies.KompasAPI.include}",
        "%{localDependencies.KompasAPI.lib}",
        "%{localDependencies.ksapi.include}",

        "%{vcpkg.include}",
    }

    links {
        "oglwrap",
        "kapiwrap",
        "generic",
        "core",

        "opengl32.lib",
        
        "%{vcpkg.lib.release}/glad.lib",

        "%{localDependencies.KompasAPI.lib64}/kApi2D5.lib",
        "%{localDependencies.KompasAPI.lib64}/kAPI3D5.lib",
        "%{localDependencies.ksapi.lib}/ksAPI.lib",
    }

    runtime "Release"

    prebuildcommands {
        "%{paths.KompasDevutil}/delete-lib.exe \"Подготовка к FDM 3D печати\""
    }

    postbuildcommands {
        "{COPYFILE} %{prj.location}/%{prj.name}.xml %{cfg.buildtarget.directory}",
        "{COPYDIR} %{vcpkg.bin.release} %{cfg.buildtarget.directory}",
        "%{paths.KompasDevutil}/add-lib.exe %{cfg.buildtarget.directory}/%{cfg.buildtarget.name}",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        defines { "NDEBUG" }
        symbols "Off"
        optimize "On"

project "kompas-print3d-optimizer"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetextension (".rtw")
    characterset ("Unicode")
    mfc "On"

    pchheader "stdafx.h"
	pchsource "src/stdafx.cpp"

    files { "**.hpp", "**.h", "**.cpp", "**.rc", "**.def" }

    forceincludes  { "stdafx.h" }

    includedirs {
        "src",
        "%{wks.location}/oglwrap/include",
        "%{wks.location}/kapiwrap/include",
        "%{wks.location}/generic/include",
        "%{wks.location}/core/include",

        "%{localDependencies.KompasAPI.include}",
        "%{localDependencies.KompasAPI.lib}",

        "%{vcpkg.include}",
    }

    links {
        "oglwrap",
        "kapiwrap",
        "generic",
        "core",

        "%{localDependencies.KompasAPI.lib64}/kApi2D5.lib",
        "%{localDependencies.KompasAPI.lib64}/kAPI3D5.lib",

        "opengl32.lib"
    }

    prebuildcommands {
        "%{paths.KompasDevutil}/delete-lib.exe \"Подготовка к FDM 3D печати\""
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        runtime "Debug"

        links {
            "%{vcpkg.lib.debug}/glad.lib",
        }

        postbuildcommands {
            "{COPYFILE} %{prj.location}/%{prj.name}.xml %{cfg.buildtarget.directory}",
            "{COPYDIR} %{vcpkg.bin.debug} %{cfg.buildtarget.directory}",
            "%{paths.KompasDevutil}/add-lib.exe %{cfg.buildtarget.directory}/%{cfg.buildtarget.name}",
        }

    filter "configurations:Release"
        defines { "NDEBUG" }
        symbols "Off"
        optimize "On"
        runtime "Release"

        links {
            "%{vcpkg.lib.release}/glad.lib",
        }

        postbuildcommands {
            "{COPYFILE} %{prj.location}/%{prj.name}.xml %{cfg.buildtarget.directory}",
            "{COPYDIR} %{vcpkg.bin.release} %{cfg.buildtarget.directory}",
            "%{paths.KompasDevutil}/add-lib.exe %{cfg.buildtarget.directory}/%{cfg.buildtarget.name}",
        }

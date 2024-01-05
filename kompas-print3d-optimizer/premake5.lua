project "kompas-print3d-optimizer"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"

    targetextension (".rtw")
    characterset ("Unicode")
    flags { "MFC" }

    pchheader "stdafx.h"
	pchsource "src/stdafx.cpp"

    files { "**.hpp", "**.h", "**.cpp", "**.rc", "**.def" }

    forceincludes  { "stdafx.h" }

    includedirs {
        "src",
        "%{wks.location}/oglwrap/include",
        "%{wks.location}/kapiwrap/include",

        "%{localDependencies.kompasApi.include}",
        "%{localDependencies.kompasApi.lib}",

        "%{vcpkg.include}",
    }

    links {
        "oglwrap",
        "kapiwrap",

        "%{localDependencies.kompasApi.lib64}/kApi2D5.lib",
        "%{localDependencies.kompasApi.lib64}/kAPI3D5.lib",

        "opengl32.lib"
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        runtime "Debug"

        links {
            "%{vcpkg.lib.debug}/glad.lib"
        }

    filter "configurations:Release"
        defines { "NDEBUG" }
        symbols "Off"
        optimize "On"
        runtime "Release"

        links {
            "%{vcpkg.lib.release}/glad.lib"
        }

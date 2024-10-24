project "kapiwrap"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    characterset ("Unicode")
    flags { "MFC" }

    pchheader "stdafx.h"
	pchsource "src/stdafx.cpp"

    files { "**.hpp", "**.h", "**.cpp", }

    forceincludes  { "stdafx.h" }

    includedirs {
        "include/kapiwrap",

        "%{localDependencies.KompasAPI.include}",
        "%{localDependencies.KompasAPI.lib}",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        optimize "Off"
        runtime "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        symbols "Off"
        optimize "On"
        runtime "Release"

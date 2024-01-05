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

        "%{wks.location}/oglwrap/include",

        "%{localDependencies.kompasApi.include}",
        "%{localDependencies.kompasApi.lib}",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        runtime "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        symbols "Off"
        optimize "On"
        runtime "Release"

project "kapiwrap"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    characterset ("Unicode")
    mfc "On"

    pchheader "stdafx.h"
	pchsource "src/stdafx.cpp"

    files { "**.hpp", "**.h", "**.cpp", }

    forceincludes  { "stdafx.h" }

    includedirs {
        "include/kapiwrap",
        "%{wks.location}/generic/include",

        "%{vcpkg.include}",

        "%{localDependencies.KompasAPI.include}",
        "%{localDependencies.KompasAPI.lib}",
        "%{localDependencies.ksapi.include}",
    }

    links {
        "generic",
    }

    runtime "Release"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        defines { "NDEBUG" }
        symbols "Off"
        optimize "On"

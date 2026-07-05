project "oglwrap"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    files { "**.hpp", "**.cpp" }

    includedirs {
        "include/oglwrap",

        "%{wks.location}/generic/include",
        "%{vcpkg.include}",
    }

    links {
        "generic"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        symbols "Off"
        optimize "On"

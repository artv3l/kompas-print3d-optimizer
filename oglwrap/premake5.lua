project "oglwrap"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    files { "**.hpp", "**.cpp" }

    includedirs {
        "include",

        "%{wks.location}/generic/include",
        "%{vcpkg.include}",
    }

    links {
        "generic"
    }

    runtime "Release"

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        symbols "Off"
        optimize "On"

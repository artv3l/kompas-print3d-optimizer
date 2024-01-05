project "oglwrap"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    files { "**.hpp", "**.cpp" }

    includedirs {
        "include/oglwrap",

        "%{vcpkg.include}",
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        symbols "Off"
        optimize "On"

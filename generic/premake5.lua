project "generic"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    files { "src/**", "include/**" }

    includedirs {
        "include/generic",
        "%{vcpkg.include}",
    }

    runtime "Release"

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        symbols "Off"
        optimize "On"

project "generic-tests"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    files { "tests/**" }

    includedirs {
        "include",
        "%{vcpkg.include}",
    }

    links {
        "generic",

        "%{vcpkg.lib.release}/gtest.lib",
        "%{vcpkg.lib.release}/gmock.lib",
    }

    runtime "Release"

    postbuildcommands {
        "{COPYDIR} %{vcpkg.bin.release} %{cfg.buildtarget.directory}",
    }

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        symbols "Off"
        optimize "On"

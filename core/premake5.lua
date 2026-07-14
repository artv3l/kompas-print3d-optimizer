project "core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    files { "src/**", "include/**" }

    includedirs {
        "include/core",
        "%{wks.location}/generic/include",
        "%{vcpkg.include}",
    }

    runtime "Release"

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        symbols "Off"
        optimize "On"

project "core-tests"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    files { "tests/**" }

    includedirs {
        "include",
        "%{wks.location}/generic/include",
        "%{vcpkg.include}",
    }

    links {
        "generic",
        "core",

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

project "generic"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    files { "src/**", "include/**" }

    includedirs {
        "include/generic",
        "%{vcpkg.include}",
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
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
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

        links {
            "%{vcpkg.lib.debug}/gtest.lib",
            "%{vcpkg.lib.debug}/gmock.lib",
        }

        postbuildcommands {
            "{COPYDIR} %{vcpkg.bin.debug} %{cfg.buildtarget.directory}",
        }
    
    filter "configurations:Release"
        runtime "Release"
        symbols "Off"
        optimize "On"

        links {
            "%{vcpkg.lib.release}/gtest.lib",
            "%{vcpkg.lib.release}/gmock.lib",
        }

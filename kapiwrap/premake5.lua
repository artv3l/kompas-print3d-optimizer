project "kapiwrap"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    files { "**.hpp", "**.cpp", }

    includedirs {
        "include",
        "%{wks.location}/generic/include",

        "%{vcpkg.include}",

        "%{localDependencies.ksapi.include}",
        "%{wks.location}/third_party/microstl-ec3868a14d8eff40f7945b39758edf623f609b6f/include",
    }

    links {
        "generic",
    }

    runtime "Release"

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        symbols "Off"
        optimize "On"

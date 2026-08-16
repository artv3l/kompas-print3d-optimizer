project "kompas-print3d-optimizer"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetextension (".rtw")

    files { "**.hpp", "**.cpp" }

    includedirs {
        "src",
        "%{wks.location}/oglwrap/include",
        "%{wks.location}/kapiwrap/include",
        "%{wks.location}/generic/include",
        "%{wks.location}/core/include",

        "%{localDependencies.ksapi.include}",

        "%{vcpkg.include}",
    }

    links {
        "oglwrap",
        "kapiwrap",
        "generic",
        "core",

        "opengl32.lib",
        "%{vcpkg.lib.release}/glad.lib",

        "%{localDependencies.ksapi.lib}/ksAPI.lib",
    }

    runtime "Release"

    postbuildcommands {
        "{COPYFILE} %{prj.location}/%{prj.name}.xml %{cfg.buildtarget.directory}",
        "{COPYDIR} %{vcpkg.bin.release} %{cfg.buildtarget.directory}",
    }

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        symbols "Off"
        optimize "On"

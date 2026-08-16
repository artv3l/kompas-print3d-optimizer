paths = {
    vcpkg = "%{wks.location}/vcpkg_installed/x64-windows",
    kompasSdk = os.getenv("KOMPAS_SDK"),
}

vcpkg = {
    include = "%{paths.vcpkg}/include",
    lib = { debug = "%{paths.vcpkg}/debug/lib", release = "%{paths.vcpkg}/lib"},
    bin = { debug = "%{paths.vcpkg}/debug/bin", release = "%{paths.vcpkg}/bin"},
}

localDependencies = {
    ksapi = {
        include = "%{paths.kompasSdk}/KsAPI/Include",
        lib = "%{paths.kompasSdk}/KsAPI/Lib64",
    },
}

workspace "kompas-print3d-optimizer"
    architecture "x86_64"
    configurations { "Debug", "Release" }
    startproject "kompas-print3d-optimizer"

include "oglwrap"
include "kapiwrap"
include "kompas-print3d-optimizer"
include "generic"
include "core"

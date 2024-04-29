vcpkgPath = "%{wks.location}/vcpkg_installed/x64-windows"
vcpkg = {
    include = "%{vcpkgPath}/include",
    lib = { debug = "%{vcpkgPath}/debug/lib", release = "%{vcpkgPath}/lib"},
    bin = { debug = "%{vcpkgPath}/debug/bin", release = "%{vcpkgPath}/bin"},
}

KOMPAS_SDK = os.getenv("KOMPAS_SDK")
localDependencies = {
    kompasApi = {
        include = "%{KOMPAS_SDK}/Include",
        lib = "%{KOMPAS_SDK}/lib",
        lib64 = "%{KOMPAS_SDK}/lib64",
    },
}

KOMPAS_DEVUTIL = os.getenv("KOMPAS_DEVUTIL")

workspace "kompas-print3d-optimizer"
    architecture "x86_64"
    configurations { "Debug", "Release" }
    startproject "kompas-print3d-optimizer"

include "oglwrap"
include "kapiwrap"
include "kompas-print3d-optimizer"
include "kp3do-test"

paths = {
    vcpkg = "%{wks.location}/vcpkg_installed/x64-windows",
    KompasSdk = os.getenv("KOMPAS_SDK"),
    KompasDevutil = os.getenv("KOMPAS_DEVUTIL"),
    SmallFbx = os.getenv("SMALL_FBX"),
    Kapiwrap = os.getenv("kapiwrap"),
}

vcpkg = {
    include = "%{paths.vcpkg}/include",
    lib = { debug = "%{paths.vcpkg}/debug/lib", release = "%{paths.vcpkg}/lib"},
    bin = { debug = "%{paths.vcpkg}/debug/bin", release = "%{paths.vcpkg}/bin"},
}

localDependencies = {
    KompasAPI = {
        include = "%{paths.KompasSdk}/Include",
        lib = "%{paths.KompasSdk}/lib",
        lib64 = "%{paths.KompasSdk}/lib64",
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

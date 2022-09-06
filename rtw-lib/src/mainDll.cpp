#include "stdafx.h"

#include <afxdllx.h>

static AFX_EXTENSION_MODULE dll = { NULL, NULL };
HINSTANCE g_hInstance = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
    g_hInstance = hInstance;
    if (dwReason == DLL_PROCESS_ATTACH) {
        TRACE0("kompas-print3d-optimizer.AWX Initializing!\n");
        AfxInitExtensionModule(dll, hInstance);
        new CDynLinkLibrary(dll);
    } else if (dwReason == DLL_PROCESS_DETACH) {
        TRACE0("kompas-print3d-optimizer.AWX Terminating!\n");
        AfxTermExtensionModule(dll);
    }
    return 1;
}

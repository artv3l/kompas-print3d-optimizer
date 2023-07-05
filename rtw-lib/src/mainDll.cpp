#include "stdafx.h"

#include <afxdllx.h>

#include "settings/SettingsManager.hpp"

extern SettingsManager settingsManager;

static AFX_EXTENSION_MODULE dll = { NULL, NULL };
HINSTANCE g_hInstance = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
    g_hInstance = hInstance;
    if (dwReason == DLL_PROCESS_ATTACH) {
        AfxInitExtensionModule(dll, hInstance);
        new CDynLinkLibrary(dll);
    } else if (dwReason == DLL_PROCESS_DETACH) {
        settingsManager.hide();
        AfxTermExtensionModule(dll);
    }
    return 1;
}

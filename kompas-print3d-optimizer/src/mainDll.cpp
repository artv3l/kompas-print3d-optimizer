#include <afxdllx.h>
#include <consoleapi.h>
#include <stdio.h>
#include <iostream>

#include "global.hpp"

static AFX_EXTENSION_MODULE dll = {NULL, NULL};

void createDebugConsole() {
    AllocConsole();
    SetConsoleTitle(L"kompas-print3d-optimizer debug console");
    FILE* fileCout;
    freopen_s(&fileCout, "CONOUT$", "w", stdout);
    freopen_s(&fileCout, "CONOUT$", "w", stderr);
}

extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        AfxInitExtensionModule(dll, hInstance);
        new CDynLinkLibrary(dll);

        global::init();

    } else if (dwReason == DLL_PROCESS_DETACH) {
        global::settingsManager->hide();
        AfxTermExtensionModule(dll);
    }
    return 1;
}

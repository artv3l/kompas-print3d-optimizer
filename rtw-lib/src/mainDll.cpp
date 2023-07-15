#include "stdafx.h"

#include <afxdllx.h>
#include <consoleapi.h>
#include <stdio.h>
#include <iostream>

#include "glad/glad.h"

#include "settings/SettingsManager.hpp"

extern SettingsManager settingsManager;

static AFX_EXTENSION_MODULE dll = { NULL, NULL };
HINSTANCE g_hInstance = NULL;

void* GetAnyGLFuncAddress(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void*)GetProcAddress(module, name);
    }
    return p;
}

extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
    g_hInstance = hInstance;
    if (dwReason == DLL_PROCESS_ATTACH) {
        AfxInitExtensionModule(dll, hInstance);
        new CDynLinkLibrary(dll);

#ifdef DEBUG
        AllocConsole();
        SetConsoleTitle(L"kompas-print3d-optimizer debug console");
        FILE* fileCout;
        freopen_s(&fileCout, "CONOUT$", "w", stdout);
        freopen_s(&fileCout, "CONOUT$", "w", stderr);
#endif // DEBUG

        if (!gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress)) {
            std::cerr << "Failed to initialize GLAD" << "\n";
            return 2;
        }

    } else if (dwReason == DLL_PROCESS_DETACH) {
        settingsManager.hide();
        AfxTermExtensionModule(dll);
    }
    return 1;
}

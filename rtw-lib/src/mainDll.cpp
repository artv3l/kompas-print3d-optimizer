#include "stdafx.h"

#include <afxdllx.h>
#include <consoleapi.h>
#include <stdio.h>
#include <iostream>
#include <stdexcept>

#include "glad/glad.h"

#include "glutil/Shader.hpp"
#include "shaders.hpp"

#include "settings/SettingsManager.hpp"

static AFX_EXTENSION_MODULE dll = {NULL, NULL};

extern SettingsManager settingsManager;

void* GetAnyGLFuncAddress(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void*)GetProcAddress(module, name);
    }
    return p;
}

ShaderProgram::Ptr shaderProgram = nullptr;

bool initShaders() {
    try {
        Shader vertexShader(VERTEX_SHADER_CODE, GL_VERTEX_SHADER);
        Shader fragmentShader(FRAGMENT_SHADER_CODE, GL_FRAGMENT_SHADER);
        shaderProgram = ShaderProgram::link({&vertexShader, &fragmentShader});
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << "\n";
        return false;
    }
    return true;
}

extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
#ifdef DEBUG
        AllocConsole();
        SetConsoleTitle(L"kompas-print3d-optimizer debug console");
        FILE* fileCout;
        freopen_s(&fileCout, "CONOUT$", "w", stdout);
        freopen_s(&fileCout, "CONOUT$", "w", stderr);
#endif // DEBUG

        AfxInitExtensionModule(dll, hInstance);
        new CDynLinkLibrary(dll);

        if (!gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress)) {
            std::cerr << "Failed to initialize GLAD" << "\n";
            return 0;
        }
        if (!initShaders()) {
            return 0;
        }

    } else if (dwReason == DLL_PROCESS_DETACH) {
        settingsManager.hide();
        AfxTermExtensionModule(dll);
    }
    return 1;
}

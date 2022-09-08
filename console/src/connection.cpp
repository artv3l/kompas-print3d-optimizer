#include "stdafx.h"
#include "connection.hpp"

#include <Windows.h>
#include <iostream>

const wchar_t objectName[] = L"KOMPAS.Application.5";

bool isKompasInstalled()
{
    CLSID clsid;
    HRESULT res;
    res = CLSIDFromProgID(objectName, &clsid);
    return (res == S_OK);
}

bool isKompasRun()
{
    CLSID clsid;
    CLSIDFromProgID(objectName, &clsid);
    HRESULT res;
    IUnknown* pIUnknown;
    res = GetActiveObject(clsid, NULL, &pIUnknown);
    if (res == S_OK) {
        pIUnknown->Release();
        return true;
    }
    return false;
}

KompasObjectPtr kompasInit() {
    if (!isKompasInstalled()) {
        std::cerr << "Компас не установлен" << "\n";
        return nullptr;
    }
    KompasObjectPtr kompas;
    if (isKompasRun()) {
        kompas.GetActiveObject(objectName);
    } else {
        kompas.CreateInstance(objectName);
    }
    kompas->Visible = true;
    return kompas;
}

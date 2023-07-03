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
        throw std::runtime_error("Kompas-3D is not installed");
    }
    KompasObjectPtr kompas;
    if (isKompasRun()) {
        kompas.GetActiveObject(objectName);
    } else {
        throw std::runtime_error("Kompas-3D is not running");
    }
    kompas->Visible = true;
    return kompas;
}

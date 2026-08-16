#include "kapiwrap/connection.hpp"

#include <stdexcept>

#include <Windows.h>


const wchar_t objectName[] = L"KOMPAS.Application.5";


bool isKompasInstalled() {
    CLSID clsid;
    HRESULT res;
    res = CLSIDFromProgID(objectName, &clsid);
    return (res == S_OK);
}

bool isKompasRun() {
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

kapi::KompasObjectPtr getRunningKompas() {
    if (!isKompasInstalled()) {
        throw std::runtime_error("Kompas-3D is not installed");
    }
    kapi::KompasObjectPtr pKompas;
    if (isKompasRun()) {
        pKompas.GetActiveObject(objectName);
    }
    else {
        throw std::runtime_error("Kompas-3D is not running");
    }
    pKompas->Visible = true;
    return pKompas;
}

kapi::KompasObjectPtr getKompasObjectPtr() {
    kapi::KompasObjectPtr getKompas(NULL);
    CString filename;
    if (::GetModuleFileName(NULL, filename.GetBuffer(255), 255)) {
        filename.ReleaseBuffer(255);
        CString libname = "kAPI5.dll";
        filename.Replace(filename.Right(filename.GetLength() - (filename.ReverseFind('\\') + 1)), libname);

        HINSTANCE hAppAuto = LoadLibrary(filename); // идентификатор kAPI5.dll
        if (hAppAuto) {
            typedef LPDISPATCH(WINAPI* FCreateKompasObject)();
            FCreateKompasObject pCreateKompasObject = (FCreateKompasObject)GetProcAddress(hAppAuto, "CreateKompasObject");
            if (pCreateKompasObject) {
                getKompas = IDispatchPtr(pCreateKompasObject(), false);
            }
            FreeLibrary(hAppAuto);
        }
    }
    return getKompas;
}

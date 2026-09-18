#include <KsAPI.h>
#include <KompasLibraryActions.h>

#include "settings/DocumentsManager.hpp"
#include "global.hpp"
#include "resources.hpp"
#include "process/OrientationSearch.hpp"

void RunCommand(unsigned int commandId, ksapi::ksRunCommandModeEnum mode)
{
    ksapi::IKompasDocument3DPtr activeDocument = Global::kompasApp->GetActiveDocument();

    if (!activeDocument) {
        Global::kompasApp->ShowMessageBox(L"Необходимо открыть 3D документ", L"Ошибка", ksMessageTypeEnum::ksMessageError,
            ksMessageButtonSetEnum::ksButtonSetOk, true /*showModal*/);
        return;
    }

    DocumentData& documentData = Global::documentsManager->getOrCreateDocumentData(activeDocument);

    switch (commandId) { 
        case 1: { // Определение плоскости печати
            OrientationSearch orientationSearch(*Global::kompasApp, activeDocument, resources::libraryName, documentData);
            orientationSearch.run();
            return;
        }
    }
}

APP_EXP_FUNC(bool) LoadKompasLibrary(ksapi::IApplication& app, ksapi::IKompasLibraryActions& libaryActions)
{
    libaryActions.AddRunCommandHandler(RunCommand);

    Global::kompasApp = &app;
    Global::init();

    return true;
}

APP_EXP_FUNC(void) UnloadKompasLibrary()
{
}

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdexcept>
#include <utility>

#include <KsAPI.h>
#include <KompasLibraryActions.h>

#include "kapiwrap/process/Process3D.hpp"

#include "settings/DocumentsManager.hpp"
#include "global.hpp"
#include "resources.hpp"
#include "process/OrientationSearch.hpp"

void RunCommand(unsigned int commandId, ksapi::ksRunCommandModeEnum mode)
{
    ksapi::IKompasDocument3DPtr activeDocument = global::kompasApp->GetActiveDocument();

    if (!activeDocument) {
        //global::kompas->ksMessage("Необходимо открыть документ-модель");
        return;
    }

    DocumentData& documentData = global::documentsManager->getOrCreateDocumentData(activeDocument);

    switch (commandId) { 
        case 1: { // Определение плоскости печати
            OrientationSearch orientationSearch(*global::kompasApp, activeDocument, resources::c_libraryName, documentData);
            orientationSearch.run();
            return;
        }
    }
}

APP_EXP_FUNC(bool) LoadKompasLibrary(ksapi::IApplication& app, ksapi::IKompasLibraryActions& libaryActions)
{
    libaryActions.AddRunCommandHandler(RunCommand);

    global::kompasApp = &app;
    global::init();

    return true;
}

APP_EXP_FUNC(void) UnloadKompasLibrary()
{
}

#include "global.hpp"

#include <memory>

#include "kapiwrap/connection.hpp"
#include "settings/DocumentsManager.hpp"
#include "settings/SettingsManager.hpp"
#include "orientation/PrFindOrientation.hpp"

kapi::KompasObjectPtr Global::kompas = nullptr;
ksapi::IApplication* Global::kompasApp = nullptr;
std::unique_ptr<DocumentsManager> Global::documentsManager = nullptr;
std::unique_ptr<SettingsManager> Global::settingsManager = nullptr;
std::unique_ptr<PrFindOrientation> Global::prFindOrientation = nullptr;

void Global::init() {
    if (!isInited()) {
        kompas = getKompasObjectPtr();
        documentsManager = std::make_unique<DocumentsManager>(kompas);
        settingsManager = std::make_unique<SettingsManager>(kompas);
    }
}

bool Global::isInited() {
    return kompas && kompasApp && documentsManager && settingsManager;
}

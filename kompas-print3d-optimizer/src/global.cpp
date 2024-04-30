#include "global.hpp"

#include <memory>

#include "kapiwrap/connection.hpp"
#include "settings/DocumentsManager.hpp"
#include "settings/SettingsManager.hpp"

kapi::KompasObjectPtr Global::kompas = nullptr;
std::unique_ptr<DocumentsManager> Global::documentsManager = nullptr;
std::unique_ptr<SettingsManager> Global::settingsManager = nullptr;

void Global::init() {
    if (!isInited()) {
        kompas = getKompasObjectPtr();
        documentsManager = std::make_unique<DocumentsManager>(kompas);
        settingsManager = std::make_unique<SettingsManager>(kompas);
    }
}

bool Global::isInited() {
    return kompas && documentsManager && settingsManager;
}

void kompasMessage(std::string message) {
#ifndef TEST_BUILD
    global::kompas->ksMessage(message.c_str());
#endif /* TEST_BUILD */
}

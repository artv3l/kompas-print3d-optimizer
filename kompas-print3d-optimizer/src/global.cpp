#include "global.hpp"

#include <memory>

#include "settings/DocumentsManager.hpp"

ksapi::IApplication* Global::kompasApp = nullptr;
std::unique_ptr<DocumentsManager> Global::documentsManager = nullptr;

void Global::init() {
    if (!isInited()) {
        documentsManager = std::make_unique<DocumentsManager>();
    }
}

bool Global::isInited() {
    return kompasApp && documentsManager;
}

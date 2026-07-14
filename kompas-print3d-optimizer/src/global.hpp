#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <memory>

#include <KsAPI.h>

#include "settings/DocumentsManager.hpp"
#include "settings/SettingsManager.hpp"
#include "orientation/PrFindOrientation.hpp"

class Global {
public:
    static kapi::KompasObjectPtr kompas;
    static ksapi::IApplication* kompasApp;

    static std::unique_ptr<DocumentsManager> documentsManager;
    static std::unique_ptr<SettingsManager> settingsManager;

    static std::unique_ptr<PrFindOrientation> prFindOrientation;

    static void init();
    static bool isInited();
};

using global = Global;

#endif /* GLOBAL_HPP */

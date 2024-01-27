#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <memory>

#include "settings/DocumentsManager.hpp"
#include "settings/SettingsManager.hpp"

class Global {
public:
    static KompasObjectPtr kompas;
    static std::unique_ptr<DocumentsManager> documentsManager;
    static std::unique_ptr<SettingsManager> settingsManager;

    static void init();
    static bool isInited();
};

using global = Global;

#endif /* GLOBAL_HPP */

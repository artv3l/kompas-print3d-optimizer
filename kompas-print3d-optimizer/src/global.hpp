#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <memory>

#include <KsAPI.h>

#include "settings/DocumentsManager.hpp"

class Global {
public:
    static ksapi::IApplication* kompasApp;

    static std::unique_ptr<DocumentsManager> documentsManager;

    static void init();
    static bool isInited();
};

using global = Global;

#endif /* GLOBAL_HPP */

#pragma once

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

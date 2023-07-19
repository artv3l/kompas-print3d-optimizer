#include "stdafx.h"
#include "global.hpp"

#include "connection.hpp"
#include "settings/DocumentsManager.hpp"
#include "settings/SettingsManager.hpp"

namespace global {
    KompasObjectPtr kompas = getKompasObjectPtr();
    DocumentsManager documentsManager(kompas);
    SettingsManager settingsManager(kompas);
}

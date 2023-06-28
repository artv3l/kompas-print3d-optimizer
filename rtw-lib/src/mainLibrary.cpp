#include "stdafx.h"

#include <sstream>
#include <unordered_map>
#include <stdexcept>

#include "resource.h"
#include "connection.hpp"

#include "PrintSurface.hpp"
#include "optimization/rounding.hpp"
#include "optimization/elephantFoot.hpp"
#include "optimization/bridgeHole.hpp"
#include "optimization/roundingEdgesOnPrintFace.hpp"
#include "optimization/circleHorizontalHoles.hpp"

#include "SettingsManager.hpp"

KompasObjectPtr kompas = getKompasObjectPtr();
SettingsManager settingsManager(kompas);

unsigned int WINAPI LIBRARYID() {
    return IDR_LIBID;
}

void WINAPI LIBRARYENTRY(unsigned int comm) {
    ksDocument3DPtr document3d = kompas->ActiveDocument3D();
    switch (comm) {
    case 1:
        settingsManager.show();
        return;
    case 2:
        try {
            PrintSurface printSurface = getSelectedPrintSurface(document3d);
            settingsManager.setPrintSurface(document3d, printSurface);
            kompas->ksMessage("Плоскость печати успешно выбрана!");
        } catch (const std::runtime_error& e) {
            kompas->ksMessage(e.what());
        }
        return;
    }

    Settings* settings = settingsManager.getSettings(document3d);
    if (!settings->printSurface) {
        kompas->ksMessage("Плоскость печати не выбрана!");
        return;
    }

    ksPartPtr part = document3d->GetPart(pTop_Part);
    size_t count = 0;

    switch (comm) {
    case 3:
    {
        ksChooseMngPtr chooseMng(document3d->GetChooseMng());
        chooseMng->UnChooseAll();
        chooseMng->Choose(settings->printSurface.value().face);
        return;
    }
    case 4:
        count += optimizeRounding(part, *settings);
        break;
    case 5:
        count += optimizeElephantFoot(part, *settings);
        break;
    case 6:
        count += optimizeRoundingEdgesOnPrintFace(kompas, part, *settings, ReworkType::ALL);
        break;
    case 7:
        count += optimizeRoundingEdgesOnPrintFace(kompas, part, *settings, ReworkType::ONLY_WITHOUT_REWORK);
        break;
    case 8:
        count += optimizeBridgeHoleFill(kompas, document3d, part, *settings, HoleType::NOT_CIRCLE);
        count += optimizeBridgeHoleBuild(kompas, document3d, part, *settings);
        break;
    case 9:
        count += optimizeBridgeHoleFill(kompas, document3d, part, *settings, HoleType::ALL);
        break;
    case 10:
        count += optimizeBridgeHoleBuild(kompas, document3d, part, *settings);
        break;
    case 11:
        count += optimizeCircleHorizontalHoles(kompas, document3d, part, *settings);
        break;
    }
    if (count == 0) {
        kompas->ksMessage("Не найдено геометрии для оптимизации");
    } else {
        document3d->RebuildDocument();
        kompas->ksMessage("Оптимизация модели была выполнена!");
    }
}

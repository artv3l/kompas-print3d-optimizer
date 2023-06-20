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
std::unordered_map<ksDocument3D*, PrintSurface> mapPrintSurface;


bool printPlaneSelectedInCurrentDocument() {
    ksDocument3DPtr currentDocument3d = kompas->ActiveDocument3D();
    std::unordered_map<ksDocument3D*, PrintSurface>::const_iterator it = mapPrintSurface.find(currentDocument3d);
    return (it != mapPrintSurface.end());
}


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
            std::unordered_map<ksDocument3D*, PrintSurface>::iterator it = mapPrintSurface.find(document3d);
            if (it == mapPrintSurface.end()) {
                mapPrintSurface.insert(std::make_pair(document3d, printSurface));
            } else {
                it->second = printSurface;
            }
            kompas->ksMessage("Плоскость печати успешно выбрана!");
        } catch (const std::runtime_error& e) {
            kompas->ksMessage(e.what());
        }
        return;
    }

    if (!printPlaneSelectedInCurrentDocument()) {
        kompas->ksError("Плоскость печати не выбрана!");
        return;
    }
    
    PrintSurface printSurface = mapPrintSurface.find(document3d)->second;
    ksPartPtr part = document3d->GetPart(pTop_Part);

    Settings settings = settingsManager.getSettings();

    switch (comm) {
    case 3: {
        ksChooseMngPtr chooseMng(document3d->GetChooseMng());
        chooseMng->UnChooseAll();
        chooseMng->Choose(printSurface.face);
        return;
    }
    case 4: {
        optimizeRounding(part, printSurface.face, settings);
        break;
    }
    case 5:
        optimizeElephantFoot(part, printSurface, settings);
        break;
    case 6:
        optimizeRoundingEdgesOnPrintFace(kompas, part, printSurface,
                                         settings, ReworkType::ALL);
        break;
    case 7:
        optimizeRoundingEdgesOnPrintFace(kompas, part, printSurface,
                                         settings, ReworkType::ONLY_WITHOUT_REWORK);
        break;
    case 8:
        optimizeBridgeHoleFill(kompas, document3d, part, printSurface.face,
                               settings, HoleType::NOT_CIRCLE);
        optimizeBridgeHoleBuild(kompas, document3d, part, printSurface.face,
                                settings);
        break;
    case 9:
        optimizeBridgeHoleFill(kompas, document3d, part, printSurface.face,
                               settings, HoleType::ALL);
        break;
    case 10:
        optimizeBridgeHoleBuild(kompas, document3d, part, printSurface.face,
                                settings);
        break;
    case 11:
        optimizeCircleHorizontalHoles(kompas, 90, printSurface.face, printSurface.eq);
        break;
    }
    document3d->RebuildDocument();
    kompas->ksMessage("Оптимизация модели была выполнена!");
}

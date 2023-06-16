#include "stdafx.h"

#include <sstream>
#include <unordered_map>

#include "resource.h"
#include "connection.hpp"

#include "selectPlane.hpp"
#include "optimization/rounding.hpp"
#include "optimization/elephantFoot.hpp"
#include "optimization/bridgeHole.hpp"
#include "optimization/roundingEdgesOnPrintFace.hpp"
#include "optimization/circleHorizontalHoles.hpp"

#include "apiutil/SettingsManager.hpp"


KompasObjectPtr kompas = getKompasObjectPtr();
SettingsManager settingsManager(kompas);
std::unordered_map<ksDocument3D*, PrintPlane> mapPrintPlane;


bool printPlaneSelectedInCurrentDocument() {
    ksDocument3DPtr currentDocument3d = kompas->ActiveDocument3D();
    std::unordered_map<ksDocument3D*, PrintPlane>::const_iterator it = mapPrintPlane.find(currentDocument3d);
    return (it != mapPrintPlane.end());
}


unsigned int WINAPI LIBRARYID() {
    return IDR_LIBID;
}

void WINAPI LIBRARYENTRY(unsigned int comm) {
    ksDocument3DPtr currentDocument3d = kompas->ActiveDocument3D();
    switch (comm) {
    case 1:
        settingsManager.show();
        return;
    case 2:
        PrintPlane printPlane = getSelectedPlane(kompas);
        std::unordered_map<ksDocument3D*, PrintPlane>::iterator it = mapPrintPlane.find(currentDocument3d);
        if (it == mapPrintPlane.end()) {
            mapPrintPlane.insert(std::make_pair(currentDocument3d, printPlane));
        } else {
            it->second = printPlane;
        }
        return;
    }

    if (!printPlaneSelectedInCurrentDocument()) {
        kompas->ksError("Плоскость печати не выбрана!");
        return;
    }
    
    PrintPlane printPlane = mapPrintPlane.find(currentDocument3d)->second;
    switch (comm) {
    case 3: {
        std::ostringstream oss;
        oss << "Высота слоя: " << settingsManager.getLayerHeight() << "\n"
            << "Максимальный угол нависаний: " << settingsManager.getOverhangThreshold();
        ksChooseMngPtr chooseMng(currentDocument3d->GetChooseMng());
        chooseMng->UnChooseAll();
        chooseMng->Choose(printPlane.face);
        kompas->ksMessage(oss.str().c_str());
        return;
    }
    case 4: {
        double radius = 0.0;
        if (kompas->ksReadDouble("Радиус:", 0.5, 0.0, DBL_MAX, &radius) != 1) {
            return;
        }
        double angle = 85.0;
        if (kompas->ksReadDouble("Граничный угол: ", 85, 0.0, 90.0, &angle) != 1) {
            return;
        }
        optimizeByRounding(kompas, printPlane.face, printPlane.eq, radius, angle);
        break;
    }
    case 5:
        optimizeElephantFoot(kompas, printPlane.face, printPlane.eq, 2 * settingsManager.getLayerHeight());
        break;
    case 6:
        optimizeRoundingEdgesOnPrintFace(kompas, currentDocument3d->GetPart(pTop_Part), printPlane.face,
            settingsManager.getOverhangThreshold(), ReworkType::ALL);
        break;
    case 7:
        optimizeRoundingEdgesOnPrintFace(kompas, currentDocument3d->GetPart(pTop_Part), printPlane.face,
            settingsManager.getLayerHeight(), ReworkType::ONLY_WITHOUT_REWORK);
        break;
    case 8:
        optimizeBridgeHoleFill(currentDocument3d, currentDocument3d->GetPart(pTop_Part), printPlane.face,
            settingsManager.getLayerHeight(), HoleType::NOT_CIRCLE);
        optimizeBridgeHoleBuild(kompas, currentDocument3d, currentDocument3d->GetPart(pTop_Part), printPlane.face,
            settingsManager.getLayerHeight());
        break;
    case 9:
        optimizeBridgeHoleFill(currentDocument3d, currentDocument3d->GetPart(pTop_Part), printPlane.face,
            settingsManager.getLayerHeight(), HoleType::ALL);
        break;
    case 10:
        optimizeBridgeHoleBuild(kompas, currentDocument3d, currentDocument3d->GetPart(pTop_Part), printPlane.face,
            settingsManager.getLayerHeight());
        break;
    case 11:
        optimizeCircleHorizontalHoles(kompas, 90, printPlane.face, printPlane.eq);
        break;
    }
    currentDocument3d->RebuildDocument();
    kompas->ksMessage("Оптимизация модели была выполнена!");
}

#include "stdafx.h"

#include <sstream>

#include "resource.h"

#include "connection.hpp"
#include "selectPlane.hpp"
#include "optimizeRounding.hpp"
#include "optimizeElephantFoot.hpp"
#include "optimizeBridgeHole.hpp"
#include "optimizeRoundingEdgesOnPrintFace.hpp"
#include "optimizeCircleHorizontalHoles.hpp"

#include "SettingsManager.hpp"


KompasObjectPtr kompas = getKompasObjectPtr();
SettingsManager settingsManager(kompas);

ksFaceDefinitionPtr printFace = nullptr;
PlaneEq printPlaneEq;
ksDocument3DPtr oldDocument = nullptr;


bool checkSelectedFace(KompasObjectPtr kompas) {
    ksDocument3DPtr doc3d = kompas->ActiveDocument3D();
    return doc3d == oldDocument && printFace;
}


unsigned int WINAPI LIBRARYID() {
    return IDR_LIBID;
}

void WINAPI LIBRARYENTRY(unsigned int comm) {
    if (!kompas) {
        return;
    }

    if ((comm > 2) && !checkSelectedFace(kompas)) {
        kompas->ksError("Плоскость печати не выбрана!");
        return;
    }
    switch (comm) {
        case 1: {
              //printSettings = inputPrintSettings(kompas);
              settingsManager.show();
              return;
        }
        case 2: {
            printFace = getSelectedPlane(kompas, &printPlaneEq);
            oldDocument = kompas->ActiveDocument3D();
            return;
        }
        case 3: {
            std::ostringstream oss;
            oss << "Диаметр сопла: " << settingsManager.getNozzleDiameter() << "\n"
                << "Высота слоя: " << settingsManager.getLayerHeight() << "\n"
                << "Максимальный угол нависаний: " << settingsManager.getOverhangThreshold() << "\n"
                << "Плоскость печати выделена";
            ksChooseMngPtr chooseMng(oldDocument->GetChooseMng());
            chooseMng->UnChooseAll();
            chooseMng->Choose(printFace);
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
              optimizeByRounding(kompas, printFace, printPlaneEq, radius, angle);
              break;
        }
        case 5: {
            optimizeElephantFoot(kompas, printFace, printPlaneEq, 2 * settingsManager.getLayerHeight());
            break;
        }
        case 6: {
            optimizeRoundingEdgesOnPrintFace(kompas, oldDocument->GetPart(pTop_Part), printFace,
                settingsManager.getOverhangThreshold(), ReworkType::ALL);
            break;
        }
        case 7: {
            optimizeRoundingEdgesOnPrintFace(kompas, oldDocument->GetPart(pTop_Part), printFace,
                settingsManager.getLayerHeight(), ReworkType::ONLY_WITHOUT_REWORK);
            break;
        }
        case 8: {
            optimizeBridgeHoleFill(oldDocument, oldDocument->GetPart(pTop_Part), printFace, settingsManager.getLayerHeight(), HoleType::NOT_CIRCLE);
            optimizeBridgeHoleBuild(kompas, oldDocument, oldDocument->GetPart(pTop_Part), printFace, settingsManager.getLayerHeight());
            break;
        }
        case 9: {
            optimizeBridgeHoleFill(oldDocument, oldDocument->GetPart(pTop_Part), printFace, settingsManager.getLayerHeight(), HoleType::ALL);
            break;
        }
        case 10: {
            optimizeBridgeHoleBuild(kompas, oldDocument, oldDocument->GetPart(pTop_Part), printFace, settingsManager.getLayerHeight());
            break;
        }
        case 11: {
            optimizeCircleHorizontalHoles(kompas, 90, printFace, printPlaneEq);
            break;
        }
        case 12: {
            break;
        }
    }
    oldDocument->RebuildDocument();
    kompas->ksMessage("Оптимизация модели была выполнена!");
}

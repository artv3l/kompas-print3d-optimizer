#include "stdafx.h"

#include <sstream>
#include <unordered_map>
#include <stdexcept>
#include <utility>

#include "resource.h"
#include "connection.hpp"

#include "settings/PrintSurface.hpp"
#include "optimization/rounding.hpp"
#include "optimization/elephantFoot.hpp"
#include "optimization/bridgeHole.hpp"
#include "optimization/roundingEdgesOnPrintFace.hpp"
#include "optimization/circleHorizontalHoles.hpp"
#include "Optional.hpp"

#include "settings/DocumentsManager.hpp"
#include "settings/SettingsManager.hpp"


const char* ROOT_MACRO_NAME = "Оптимизации";

KompasObjectPtr kompas = getKompasObjectPtr();
DocumentsManager documentsManager;
SettingsManager settingsManager(kompas, documentsManager);

template <typename T>
void handleOptimizationResult(std::pair<size_t, Optional<T>> optimizationResult, Macro& rootMacro, size_t& outCount) {
    outCount += optimizationResult.first;
    if (optimizationResult.second) {
        rootMacro.add(optimizationResult.second.value());
    }
}

unsigned int WINAPI LIBRARYID() {
    return IDR_LIBID;
}

void WINAPI LIBRARYENTRY(unsigned int comm) {
    ksDocument3DPtr document3d = kompas->ActiveDocument3D();
    if (!document3d) {
        kompas->ksMessage("Необходимо открыть документ-модель");
        return;
    }

    DocumentData& documentData = documentsManager.getOrCreateDocumentData(document3d);
    DocumentData::Settings& settings = documentData.getSettings();

    switch (comm) {
    case 1:
        settingsManager.show(settings);
        return;
    case 2:
        try {
            PrintSurface printSurface = getSelectedPrintSurface(document3d);
            settings.setPrintSurface(printSurface);
            kompas->ksMessage("Плоскость печати успешно выбрана!");
        } catch (const std::runtime_error& e) {
            kompas->ksMessage(e.what());
        }
        return;
    }

    if (!settings.isPrintSurfaceSelected()) {
        kompas->ksMessage("Плоскость печати не выбрана!");
        return;
    }

    Macro rootMacro = documentData.getRootMacro();
    ksPartPtr part = document3d->GetPart(pTop_Part);
    size_t count = 0;

    switch (comm) {
    case 3: {
        ksChooseMngPtr chooseMng(document3d->GetChooseMng());
        chooseMng->UnChooseAll();
        chooseMng->Choose(settings.getPrintSurface().face);
        return;
    }
    case 4:
        handleOptimizationResult(optimizeRounding(part, settings), rootMacro, count);
        break;
    case 5:
        handleOptimizationResult(optimizeElephantFoot(part, settings), rootMacro, count);
        break;
    case 6:
        handleOptimizationResult(optimizeRoundingEdgesOnPrintFace(kompas, part, settings, ReworkType::ALL), rootMacro, count);
        break;
    case 7:
        handleOptimizationResult(optimizeRoundingEdgesOnPrintFace(kompas, part, settings, ReworkType::ONLY_WITHOUT_REWORK), rootMacro, count);
        break;
    case 8:
        handleOptimizationResult(optimizeBridgeHoleFill(kompas, document3d, part, settings, HoleType::NOT_CIRCLE), rootMacro, count);
        handleOptimizationResult(optimizeBridgeHoleBuild(kompas, document3d, part, settings), rootMacro, count);
        break;
    case 9:
        handleOptimizationResult(optimizeBridgeHoleFill(kompas, document3d, part, settings, HoleType::ALL), rootMacro, count);
        break;
    case 10:
        handleOptimizationResult(optimizeBridgeHoleBuild(kompas, document3d, part, settings), rootMacro, count);
        break;
    case 11:
        handleOptimizationResult(optimizeCircleHorizontalHoles(kompas, document3d, part, settings), rootMacro, count);
        break;
    }
    if (count == 0) {
        kompas->ksMessage("Не найдено геометрии для оптимизации");
    } else {
        document3d->RebuildDocument();
        rootMacro.update();
        kompas->ksMessage("Оптимизация модели была выполнена!");
    }
}

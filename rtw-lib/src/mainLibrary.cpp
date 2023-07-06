#include "stdafx.h"

#define _USE_MATH_DEFINES
#include <math.h>
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
#include "settings/DocumentsManager.hpp"
#include "settings/SettingsManager.hpp"
#include "settings/SettingInitializer.hpp"
#include "utils.hpp"

KompasObjectPtr kompas = getKompasObjectPtr();
DocumentsManager documentsManager;
SettingsManager settingsManager(kompas, documentsManager);

void handleOptimizationResult(std::pair<size_t, ksEntityPtr> optimizationResult, Macro& rootMacro, size_t& outCount) {
    outCount += optimizationResult.first;
    if (optimizationResult.second) {
        rootMacro.add(optimizationResult.second);
    }
}

void fastExportStl(ksDocument3DPtr document3d, DocumentData::Settings& settings) {
    ksAdditionFormatParamPtr param = document3d->AdditionFormatParam();
    param->Init();
    param->format = D3FormatConvType::format_STL;
    param->formatBinary = true;
    param->angle = 2 * M_PI / 180.0;
    param->stepType = ksStepTypeEnum::ksDeviationStep;

    std::pair<std::string, std::string> pair = splitFileNameAndRemoveExtension(std::string(document3d->fileName));
    std::string stlFolder = std::string(settings.getStringSetting(SI_EXPORT_STL_FOLDER.name)->getValue());
    
    std::string resultFolder = pair.first + "\\" + stlFolder;
    CreateDirectoryA(resultFolder.c_str(), nullptr);

    std::string result = resultFolder + "\\" + pair.second + ".stl";
    document3d->SaveAsToAdditionFormat(result.c_str(), param);
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

    switch (comm) {
    case 5:
        fastExportStl(document3d, settings);
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
    case 10:
        handleOptimizationResult(optimizeRounding(part, settings), rootMacro, count);
        break;
    case 11:
        handleOptimizationResult(optimizeElephantFoot(part, settings), rootMacro, count);
        break;
    case 12:
        handleOptimizationResult(optimizeRoundingEdgesOnPrintFace(kompas, part, settings, ReworkType::ALL), rootMacro, count);
        break;
    case 13:
        handleOptimizationResult(optimizeRoundingEdgesOnPrintFace(kompas, part, settings, ReworkType::ONLY_WITHOUT_REWORK), rootMacro, count);
        break;
    case 14:
        handleOptimizationResult(optimizeBridgeHoleFill(kompas, document3d, part, settings, HoleType::NOT_CIRCLE), rootMacro, count);
        handleOptimizationResult(optimizeBridgeHoleBuild(kompas, document3d, part, settings), rootMacro, count);
        break;
    case 15:
        handleOptimizationResult(optimizeBridgeHoleFill(kompas, document3d, part, settings, HoleType::ALL), rootMacro, count);
        break;
    case 16:
        handleOptimizationResult(optimizeBridgeHoleBuild(kompas, document3d, part, settings), rootMacro, count);
        break;
    case 17:
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

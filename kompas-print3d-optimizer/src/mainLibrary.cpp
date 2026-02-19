#define _USE_MATH_DEFINES
#include <math.h>
#include <stdexcept>
#include <utility>

#include <WinUser.h>

#include "resource.h"
#include "kapiwrap/connection.hpp"

#include "settings/PrintSurface.hpp"
#include "optimization/rounding.hpp"
#include "optimization/elephantFoot.hpp"
#include "optimization/bridgeHole.hpp"
#include "optimization/roundingEdgesOnPrintFace.hpp"
#include "optimization/circleHorizontalHoles.hpp"
#include "settings/DocumentsManager.hpp"
#include "settings/SettingsManager.hpp"
#include "orientation/PrFindOrientation.hpp"
#include "utils.hpp"
#include "global.hpp"


void fastExportStl(kapi::ksDocument3DPtr document3d, Settings& settings) {
    kapi::ksAdditionFormatParamPtr param = document3d->AdditionFormatParam();
    param->Init();
    param->format = kapi::D3FormatConvType::format_STL;
    param->formatBinary = true;
    param->angle = 2 * M_PI / 180.0;
    param->stepType = kapi::ksStepTypeEnum::ksDeviationStep;

    std::pair<std::string, std::string> pair = splitFileNameAndRemoveExtension(std::string(document3d->fileName));
    std::string stlFolder = std::string(settings.getStringSetting(si::exportStlFolder.name)->getValue());
    
    std::string resultFolder = pair.first + "\\" + stlFolder;
    CreateDirectoryA(resultFolder.c_str(), nullptr);

    std::string result = resultFolder + "\\" + pair.second + ".stl";
    document3d->SaveAsToAdditionFormat(result.c_str(), param);
}

unsigned int WINAPI LIBRARYID() {
    return IDR_LIBID;
}

void WINAPI LIBRARYENTRY(unsigned int comm) {

    kapi::ksDocument3DPtr document3d = global::kompas->ActiveDocument3D();
    if (!document3d) {
        global::kompas->ksMessage("Необходимо открыть документ-модель");
        return;
    }

    DocumentData& documentData = global::documentsManager->getOrCreateDocumentData(document3d);
    Settings* settings = documentData.getSettings();
    HighlightingManager* highlightingManager = documentData.getHighlightingManager();

    switch (comm) { // Настройки
    case 1:
        global::settingsManager->show(settings);
        return;
    case 2:
        try {
            PrintSurface printSurface = getSelectedPrintSurface(document3d);
            settings->setPrintSurface(printSurface);
            global::kompas->ksMessage("Плоскость печати успешно выбрана!");
        } catch (const std::runtime_error& e) {
            global::kompas->ksMessage(e.what());
        }
        return;
    }

    switch (comm) { // Быстрый экспорт
    case 5: {
        kapi::IApplicationPtr application = global::kompas->ksGetApplication7();
        fastExportStl(document3d, *settings);
        application->kApi7_MessageBoxEx("Сохранено в STL", "", MB_ICONINFORMATION);
        return;
    }}

    switch (comm) { // Определение плоскости печати
    case 21: {
        global::prFindOrientation = std::make_unique<PrFindOrientation>(global::kompas, documentData);
        global::prFindOrientation->show();
        return;
    }
    }

    if (!settings->isPrintSurfaceSelected()) {
        global::kompas->ksMessage("Плоскость печати не выбрана!");
        return;
    }

    if (comm >= 30) { // Подсветка элементов
        switch (comm) {
        case 30:
            highlightingManager->toggleMode(HighlightingManager::Mode::layersEverywhere);
            break;
        case 31:
            highlightingManager->toggleMode(HighlightingManager::Mode::layersAtCursor);
            break;
        case 32:
            highlightingManager->toggleMode(HighlightingManager::Mode::overhangs);
            break;
        }
        highlightingManager->refreshWindow();
        return;
    }

    // Оптимизации
    kapi::ksPartPtr part = document3d->GetPart(kapi::pTop_Part);
    kapi::ksEntityPtr optimizationResult = nullptr;
    size_t reworkCount = 0;
    switch (comm) {
    case 10:
        optimizationResult = optimizeRounding(part, *settings);
        break;
    case 11:
        optimizationResult = optimizeElephantFoot(part, *settings);
        break;
    case 12:
        optimizationResult = optimizeRoundingEdgesOnPrintFace(global::kompas, part, *settings, ReworkType::ALL, reworkCount);
        break;
    case 13:
        optimizationResult = optimizeRoundingEdgesOnPrintFace(global::kompas, part, *settings, ReworkType::ONLY_WITHOUT_REWORK, reworkCount);
        break;
    case 14:
        optimizationResult = optimizeBridgeHoleFill(global::kompas, document3d, part, *settings, HoleType::NOT_CIRCLE);
        optimizationResult = optimizeBridgeHoleBuild(global::kompas, document3d, part, *settings);
        break;
    case 15:
        optimizationResult = optimizeBridgeHoleFill(global::kompas, document3d, part, *settings, HoleType::ALL);
        break;
    case 16:
        optimizationResult = optimizeBridgeHoleBuild(global::kompas, document3d, part, *settings);
        break;
    case 17:
        optimizationResult = optimizeCircleHorizontalHoles(global::kompas, document3d, part, *settings);
        break;
    }
    if (!optimizationResult) {
        global::kompas->ksMessage("Не найдено геометрии для оптимизации");
    } else {
        Macro rootMacro = documentData.getOrCreateRootMacro();
        rootMacro.add(optimizationResult);
        document3d->RebuildDocument();
        if (reworkCount != 0) {
            global::kompas->ksMessage("Необходимо доработать элементов: " + _bstr_t(reworkCount));
        } else {
            global::kompas->ksMessage("Оптимизация модели была выполнена!");
        }
    }
}

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdexcept>
#include <utility>

#include <WinUser.h>

#include <KsAPI.h>
#include <KompasLibraryActions.h>

#include "resource.h"
#include "kapiwrap/connection.hpp"

#include "settings/PrintSurface.hpp"
#include "optimization/optimizations.hpp"
#include "optimization/bridgeHole.hpp"
#include "optimization/circleHorizontalHoles.hpp"
#include "settings/DocumentsManager.hpp"
#include "settings/SettingsManager.hpp"
#include "orientation/PrFindOrientation.hpp"
#include "global.hpp"


void fastExportStl(kapi::ksDocument3DPtr document3d, Settings& settings)
{
    auto splitFileNameAndRemoveExtension = [](std::string fileName) -> std::pair<std::string, std::string>
    {
        size_t lastSlashIndex = fileName.find_last_of('\\');
        size_t lastDotIndex = fileName.find_last_of('.');
        return std::make_pair(fileName.substr(0, lastSlashIndex), fileName.substr(lastSlashIndex + 1, lastDotIndex - lastSlashIndex - 1));
    };

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

void RunCommand(unsigned int commandId, ksapi::ksRunCommandModeEnum mode)
{
    kapi::ksDocument3DPtr document3d = global::kompas->ActiveDocument3D();
    if (!document3d) {
        global::kompas->ksMessage("Необходимо открыть документ-модель");
        return;
    }

    DocumentData& documentData = global::documentsManager->getOrCreateDocumentData(document3d);
    Settings* settings = documentData.getSettings();
    HighlightingManager* highlightingManager = documentData.getHighlightingManager();

    switch (commandId) { // Настройки
    case 1:
        global::settingsManager->show(settings);
        return;
    case 2:
        try {
            PrintSurface printSurface = getSelectedPrintSurface(document3d);
            settings->setPrintSurface(printSurface);
            global::kompas->ksMessage("Плоскость печати успешно выбрана!");
        }
        catch (const std::runtime_error& e) {
            global::kompas->ksMessage(e.what());
        }
        return;
    }

    switch (commandId) { // Быстрый экспорт
    case 5: {
        kapi::IApplicationPtr application = global::kompas->ksGetApplication7();
        fastExportStl(document3d, *settings);
        application->kApi7_MessageBoxEx("Сохранено в STL", "", MB_ICONINFORMATION);
        return;
    }
    }

    switch (commandId) { // Определение плоскости печати
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

    if (commandId >= 30) { // Подсветка элементов

        // TODO Не работает отключение подсветок

        kapi::ksPartPtr part = document3d->GetPart(kapi::Part_Type::pTop_Part);
        kapi::ksBodyPtr body = part->GetMainBody();
        auto mesh = std::make_shared<Mesh>(copyToMesh(body));
        highlightingManager->cleanObjects();
        highlightingManager->addObject(mesh, Visualizer::meshHighlight3dp);

        switch (commandId) {
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

    ksapi::IKompasDocument3DPtr activeDocument = global::kompasApp->GetActiveDocument();
    ksapi::IPartPtr topPart = activeDocument->GetTopPart();

    size_t reworkCount = 0;
    switch (commandId) {
    case 11:
        optimizeElephantFoot(topPart, *settings);
        break;
    case 12:
        optimizeRoundingEdgesOnPrintFace(topPart, *settings, ReworkType::ALL, reworkCount);
        break;
    case 13:
        optimizeRoundingEdgesOnPrintFace(topPart, *settings, ReworkType::ONLY_WITHOUT_REWORK, reworkCount);
        break;
    case 14:
        optimizeBridgeHoleFill(global::kompas, document3d, part, *settings, HoleType::NOT_CIRCLE);
        optimizeBridgeHoleBuild(global::kompas, document3d, part, *settings);
        break;
    case 15:
        optimizeBridgeHoleFill(global::kompas, document3d, part, *settings, HoleType::ALL);
        break;
    case 16:
        optimizeBridgeHoleBuild(global::kompas, document3d, part, *settings);
        break;
    case 17:
        optimizeCircleHorizontalHoles(global::kompas, document3d, part, *settings);
        break;
    }
}

APP_EXP_FUNC(bool) LoadKompasLibrary(ksapi::IApplication& app, ksapi::IKompasLibraryActions& libaryActions)
{
    libaryActions.AddRunCommandHandler(RunCommand);

    global::kompasApp = &app;
    global::init();

    return true;
}

APP_EXP_FUNC(void) UnloadKompasLibrary()
{
    global::settingsManager->hide();
    if (global::prFindOrientation)
        global::prFindOrientation->hide();
}

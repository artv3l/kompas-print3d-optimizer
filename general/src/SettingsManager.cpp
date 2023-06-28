#include "stdafx.h"
#include "SettingsManager.hpp"

#include <utility>
#include <stdexcept>

#include "apiutil/PropertyManagerObject.hpp"
#include "DocumentsManager.hpp"

const double DEFAULT_LAYER_HEIGHT = 0.2;
const double DEFAULT_OVERHANG_THRESHOLD = 45.0;
const double DEFAULT_ROUNDING_RADIUS = 1.0;
const double DEFAULT_ROUNDING_DEFLECTION_ANGLE = 5.0;
const uint8_t DEFAULT_ELEPHANT_FOOT_LAYERS_COUNT = 2;
const uint8_t DEFAULT_BRIDGE_HOLE_FILL_LAYERS_COUNT = 1;
const uint8_t DEFAULT_BRIDGE_HOLE_BUILD_LAYERS_COUNT = 1;

Settings::Settings():
    printSurface(),
    layerHeight(DEFAULT_LAYER_HEIGHT), overhangThreshold(DEFAULT_OVERHANG_THRESHOLD),
    roundingRadius(DEFAULT_ROUNDING_RADIUS), roundingDeflectionAngle(DEFAULT_ROUNDING_DEFLECTION_ANGLE),
    elephantFootLayersCount(DEFAULT_ELEPHANT_FOOT_LAYERS_COUNT),
    bridgeHoleFillLayersCount(DEFAULT_BRIDGE_HOLE_FILL_LAYERS_COUNT), bridgeHoleBuildLayersCount(DEFAULT_BRIDGE_HOLE_BUILD_LAYERS_COUNT)
{}

Settings::Settings(const PrintSurface& printSurface_):
    Settings()
{
    printSurface = printSurface_;
}

SettingsManager::SettingsManager(KompasObjectPtr kompas, DocumentsManager& documentsManager) :
    PropertyManagerObject(kompas),
    documentsManager_(documentsManager),
    mainTab_(propertyManager_->PropertyTabs->Add("MainTab")), controls_(mainTab_->PropertyControls)
{
    propertyManager_->Layout = PropertyManagerLayout::pmAlignRight;
    propertyManager_->SpecToolbar = SpecPropertyToolBarEnum::pnEnterEscHelp;
    propertyManager_->Caption = _T("Параметры печати");

    initControls();
}

void SettingsManager::show() {
    fillSettingsToEdits();
    PropertyManagerObject::show();
}

void SettingsManager::setPrintSurface(ksDocument3DPtr document3d, const PrintSurface& printSurface) {
    DocumentData* documentData = documentsManager_.getOrCreateDocumentData(document3d);
    documentData->settings.printSurface = printSurface;
}

Settings* SettingsManager::getSettings(ksDocument3DPtr document3d) {
    DocumentData* documentData = documentsManager_.getOrCreateDocumentData(document3d);
    return &documentData->settings;
}

bool SettingsManager::buttonClick(long buttonId) {
    ksDocument3DPtr document3d = kompas_->ActiveDocument3D();
    if (!document3d) {
        return false;
    }
    Settings* settings = getSettings(document3d);

    /*
        Почему-то, при чтении Value, его значение запоминается в панели автоматически (если же это значение не прочитать, то оно не сохранится).
        Поэтому заненсение значений настроек в поля при показе панели и не реализовано (это происходит само по себе).
    */
    switch (buttonId) {
    case SpecPropertyButtonEnum::pbEnter:
        settings->layerHeight = edits_.layerHeight->Value;
        settings->overhangThreshold = edits_.overhangThreshold->Value;
        settings->roundingRadius = edits_.roundingRadius->Value;
        settings->roundingDeflectionAngle = edits_.roundingDeflectionAngle->Value;
        settings->elephantFootLayersCount = edits_.elephantFootLayersCount->Value;
        settings->bridgeHoleFillLayersCount = edits_.bridgeHoleFillLayersCount->Value;
        settings->bridgeHoleBuildLayersCount = edits_.bridgeHoleBuildLayersCount->Value;

        hide();
        break;
    case SpecPropertyButtonEnum::pbHelp:
        break;
    case SpecPropertyButtonEnum::pbEsc:
        hide();
        break;
    }
    return true;
}

void SettingsManager::initControls() {
    {
        IPropertyGroupBeginPtr printSettingsGroupBegin = controls_->Add(ControlTypeEnum::ksControlGroupBegin);
        printSettingsGroupBegin->Name = "Параметры печати";
        printSettingsGroupBegin->Expanding = true;

        edits_.layerHeight = controls_->Add(ControlTypeEnum::ksControlEditReal);
        edits_.layerHeight->Name = "Высота слоя";
        edits_.layerHeight->SetValueRange(0.04, 0.4);
        edits_.layerHeight->Step = 0.04;
        edits_.layerHeight->Value = DEFAULT_LAYER_HEIGHT;

        edits_.overhangThreshold = controls_->Add(ControlTypeEnum::ksControlEditInt);
        edits_.overhangThreshold->Name = "Максимальный угол нависаний";
        edits_.overhangThreshold->SetValueRange(0, 90);
        edits_.overhangThreshold->Step = 5;
        edits_.overhangThreshold->Value = DEFAULT_OVERHANG_THRESHOLD;

        controls_->Add(ControlTypeEnum::ksControlGroupEnd); /* printSettings */
    }
    {
        IPropertyGroupBeginPtr roundingGroupBegin = controls_->Add(ControlTypeEnum::ksControlGroupBegin);
        roundingGroupBegin->Name = "Выпирающие углы";
        roundingGroupBegin->Expanding = true;

        edits_.roundingRadius = controls_->Add(ControlTypeEnum::ksControlEditReal);
        edits_.roundingRadius->Name = "Радиус";
        edits_.roundingRadius->SetValueRange(0.1, 10.0);
        edits_.roundingRadius->Step = 0.1;
        edits_.roundingRadius->Value = DEFAULT_ROUNDING_RADIUS;

        edits_.roundingDeflectionAngle = controls_->Add(ControlTypeEnum::ksControlEditInt);
        edits_.roundingDeflectionAngle->Name = "Угол отклонения";
        edits_.roundingDeflectionAngle->SetValueRange(0, 20);
        edits_.roundingDeflectionAngle->Step = 1;
        edits_.roundingDeflectionAngle->Value = DEFAULT_ROUNDING_DEFLECTION_ANGLE;

        controls_->Add(ControlTypeEnum::ksControlGroupEnd); /* rounding */
    }
    {
        IPropertyGroupBeginPtr elephantFootGroupBegin = controls_->Add(ControlTypeEnum::ksControlGroupBegin);
        elephantFootGroupBegin->Name = "Слоновья нога";
        elephantFootGroupBegin->Expanding = true;

        edits_.elephantFootLayersCount = controls_->Add(ControlTypeEnum::ksControlEditInt);
        edits_.elephantFootLayersCount->Name = "Кол-во слоев";
        edits_.elephantFootLayersCount->SetValueRange(1, 5);
        edits_.elephantFootLayersCount->Step = 1;
        edits_.elephantFootLayersCount->Value = DEFAULT_ELEPHANT_FOOT_LAYERS_COUNT;

        controls_->Add(ControlTypeEnum::ksControlGroupEnd); /* elephantFoot */
    }
    {
        IPropertyGroupBeginPtr bridgeHoleFillGroupBegin = controls_->Add(ControlTypeEnum::ksControlGroupBegin);
        bridgeHoleFillGroupBegin->Name = "Нависающие отверстия: закрытие диафрагмой";
        bridgeHoleFillGroupBegin->Expanding = true;

        edits_.bridgeHoleFillLayersCount = controls_->Add(ControlTypeEnum::ksControlEditInt);
        edits_.bridgeHoleFillLayersCount->Name = "Слоев в диафрагме";
        edits_.bridgeHoleFillLayersCount->SetValueRange(1, 5);
        edits_.bridgeHoleFillLayersCount->Step = 1;
        edits_.bridgeHoleFillLayersCount->Value = DEFAULT_BRIDGE_HOLE_FILL_LAYERS_COUNT;

        controls_->Add(ControlTypeEnum::ksControlGroupEnd); /* bridgeHoleFill */
    }
    {
        IPropertyGroupBeginPtr bridgeHoleBuildGroupBegin = controls_->Add(ControlTypeEnum::ksControlGroupBegin);
        bridgeHoleBuildGroupBegin->Name = "Нависающие отверстия: достройка до набора мостов";
        bridgeHoleBuildGroupBegin->Expanding = true;

        edits_.bridgeHoleBuildLayersCount = controls_->Add(ControlTypeEnum::ksControlEditInt);
        edits_.bridgeHoleBuildLayersCount->Name = "Слоев в мосте";
        edits_.bridgeHoleBuildLayersCount->SetValueRange(1, 5);
        edits_.bridgeHoleBuildLayersCount->Step = 1;
        edits_.bridgeHoleBuildLayersCount->Value = DEFAULT_BRIDGE_HOLE_BUILD_LAYERS_COUNT;

        controls_->Add(ControlTypeEnum::ksControlGroupEnd); /* bridgeHoleBuild */
    }
}

void SettingsManager::fillSettingsToEdits() {
    ksDocument3DPtr document3d = kompas_->ActiveDocument3D();
    if (!document3d) {
        return;
    }
    Settings* settings = getSettings(document3d);
    edits_.layerHeight->Value = settings->layerHeight;
    edits_.overhangThreshold->Value = settings->overhangThreshold;
    edits_.roundingRadius->Value = settings->roundingRadius;
    edits_.roundingDeflectionAngle->Value = settings->roundingDeflectionAngle;
    edits_.elephantFootLayersCount->Value = settings->elephantFootLayersCount;
    edits_.bridgeHoleFillLayersCount->Value = settings->bridgeHoleFillLayersCount;
    edits_.bridgeHoleBuildLayersCount->Value = settings->bridgeHoleBuildLayersCount;
}

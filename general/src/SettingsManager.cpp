#include "stdafx.h"
#include "SettingsManager.hpp"

#include <utility>

#include "apiutil/PropertyManagerObject.hpp"

const Settings DEFAULT_SETTINGS {0.2, 45, 1, 5, 2, 1, 1};

SettingsManager::SettingsManager(KompasObjectPtr kompas) :
    PropertyManagerObject(kompas),
    mainTab_(propertyManager_->PropertyTabs->Add("MainTab")), controls_(mainTab_->PropertyControls),
    settings_(DEFAULT_SETTINGS)
{
    propertyManager_->Layout = PropertyManagerLayout::pmAlignRight;
    propertyManager_->SpecToolbar = SpecPropertyToolBarEnum::pnEnterEscHelp;
    propertyManager_->Caption = _T("Параметры печати");

    {
        IPropertyGroupBeginPtr printSettingsGroupBegin = controls_->Add(ControlTypeEnum::ksControlGroupBegin);
        printSettingsGroupBegin->Name = "Параметры печати";
        printSettingsGroupBegin->Expanding = true;

        layerHeightEdit_ = controls_->Add(ControlTypeEnum::ksControlEditReal);
        layerHeightEdit_->Name = "Высота слоя";
        layerHeightEdit_->SetValueRange(0.04, 0.4);
        layerHeightEdit_->Step = 0.04;
        layerHeightEdit_->Value = DEFAULT_SETTINGS.layerHeight;

        overhangThresholdEdit_ = controls_->Add(ControlTypeEnum::ksControlEditInt);
        overhangThresholdEdit_->Name = "Максимальный угол нависаний";
        overhangThresholdEdit_->SetValueRange(0, 90);
        overhangThresholdEdit_->Step = 5;
        overhangThresholdEdit_->Value = DEFAULT_SETTINGS.overhangThreshold;

        controls_->Add(ControlTypeEnum::ksControlGroupEnd); /* printSettings */
    }
    {
        IPropertyGroupBeginPtr roundingGroupBegin = controls_->Add(ControlTypeEnum::ksControlGroupBegin);
        roundingGroupBegin->Name = "Выпирающие углы";
        roundingGroupBegin->Expanding = true;

        roundingRadiusEdit_ = controls_->Add(ControlTypeEnum::ksControlEditReal);
        roundingRadiusEdit_->Name = "Радиус";
        roundingRadiusEdit_->SetValueRange(0.1, 10.0);
        roundingRadiusEdit_->Step = 0.1;
        roundingRadiusEdit_->Value = DEFAULT_SETTINGS.roundingRadius;

        roundingDeflectionAngleEdit_ = controls_->Add(ControlTypeEnum::ksControlEditInt);
        roundingDeflectionAngleEdit_->Name = "Угол отклонения";
        roundingDeflectionAngleEdit_->SetValueRange(0, 20);
        roundingDeflectionAngleEdit_->Step = 1;
        roundingDeflectionAngleEdit_->Value = DEFAULT_SETTINGS.roundingDeflectionAngle;

        controls_->Add(ControlTypeEnum::ksControlGroupEnd); /* rounding */
    }
    {
        IPropertyGroupBeginPtr elephantFootGroupBegin = controls_->Add(ControlTypeEnum::ksControlGroupBegin);
        elephantFootGroupBegin->Name = "Слоновья нога";
        elephantFootGroupBegin->Expanding = true;

        elephantFootLayersCountEdit_ = controls_->Add(ControlTypeEnum::ksControlEditInt);
        elephantFootLayersCountEdit_->Name = "Кол-во слоев";
        elephantFootLayersCountEdit_->SetValueRange(1, 5);
        elephantFootLayersCountEdit_->Step = 1;
        elephantFootLayersCountEdit_->Value = DEFAULT_SETTINGS.elephantFootLayersCount;

        controls_->Add(ControlTypeEnum::ksControlGroupEnd); /* elephantFoot */
    }
    {
        IPropertyGroupBeginPtr bridgeHoleFillGroupBegin = controls_->Add(ControlTypeEnum::ksControlGroupBegin);
        bridgeHoleFillGroupBegin->Name = "Нависающие отверстия: закрытие диафрагмой";
        bridgeHoleFillGroupBegin->Expanding = true;

        bridgeHoleFillLayersCountEdit_ = controls_->Add(ControlTypeEnum::ksControlEditInt);
        bridgeHoleFillLayersCountEdit_->Name = "Слоев в диафрагме";
        bridgeHoleFillLayersCountEdit_->SetValueRange(1, 5);
        bridgeHoleFillLayersCountEdit_->Step = 1;
        bridgeHoleFillLayersCountEdit_->Value = DEFAULT_SETTINGS.bridgeHoleFillLayersCount;

        controls_->Add(ControlTypeEnum::ksControlGroupEnd); /* bridgeHoleFill */
    }
    {
        IPropertyGroupBeginPtr bridgeHoleBuildGroupBegin = controls_->Add(ControlTypeEnum::ksControlGroupBegin);
        bridgeHoleBuildGroupBegin->Name = "Нависающие отверстия: достройка до набора мостов";
        bridgeHoleBuildGroupBegin->Expanding = true;

        bridgeHoleBuildLayersCountEdit_ = controls_->Add(ControlTypeEnum::ksControlEditInt);
        bridgeHoleBuildLayersCountEdit_->Name = "Слоев в мосте";
        bridgeHoleBuildLayersCountEdit_->SetValueRange(1, 5);
        bridgeHoleBuildLayersCountEdit_->Step = 1;
        bridgeHoleBuildLayersCountEdit_->Value = DEFAULT_SETTINGS.bridgeHoleBuildLayersCount;

        controls_->Add(ControlTypeEnum::ksControlGroupEnd); /* bridgeHoleBuild */
    }
}

Settings SettingsManager::getSettings() const {
    return settings_;
}

bool SettingsManager::buttonClick(long buttonId) {
    /*
        Почему-то, при чтении Value, его значение запоминается в панели автоматически (если же это значение не прочитать, то оно не сохранится).
        Поэтому заненсение значений настроек в поля при показе панели и не реализовано (это происходит само по себе).
    */
    switch (buttonId) {
    case SpecPropertyButtonEnum::pbEnter:
        settings_.layerHeight = layerHeightEdit_->Value;
        settings_.overhangThreshold = overhangThresholdEdit_->Value;
        settings_.roundingRadius = roundingRadiusEdit_->Value;
        settings_.roundingDeflectionAngle = roundingDeflectionAngleEdit_->Value;
        settings_.elephantFootLayersCount = elephantFootLayersCountEdit_->Value;
        settings_.bridgeHoleFillLayersCount = bridgeHoleFillLayersCountEdit_->Value;
        settings_.bridgeHoleBuildLayersCount = bridgeHoleBuildLayersCountEdit_->Value;

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

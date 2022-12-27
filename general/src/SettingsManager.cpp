#include "stdafx.h"
#include "SettingsManager.hpp"

#include <utility>

#include "PropertyManagerObject.hpp"

SettingsManager::SettingsManager(KompasObjectPtr kompas):
        PropertyManagerObject(kompas),
        mainTab_(propertyManager_->PropertyTabs->Add("MainTab")), controls_(mainTab_->PropertyControls),
        layerHeight_(std::make_pair(nullptr, 0.2)), overhangThreshold_(std::make_pair(nullptr, 45))
{
    propertyManager_->Layout = PropertyManagerLayout::pmAlignRight;
    propertyManager_->SpecToolbar = SpecPropertyToolBarEnum::pnEnterEscHelp;
    propertyManager_->Caption = _T("Параметры печати");

    IPropertyGroupBeginPtr printSettingsGroupBegin = controls_->Add(ControlTypeEnum::ksControlGroupBegin);
    printSettingsGroupBegin->Name = "Параметры печати";
    printSettingsGroupBegin->Expanding = true;

    layerHeight_.first = controls_->Add(ControlTypeEnum::ksControlEditReal);
    layerHeight_.first->Name = "Высота слоя";
    layerHeight_.first->SetValueRange(0.04, 0.4);
    layerHeight_.first->Step = 0.04;
    layerHeight_.first->Value = layerHeight_.second;

    overhangThreshold_.first = controls_->Add(ControlTypeEnum::ksControlEditInt);
    overhangThreshold_.first->Name = "Максимальный угол нависаний";
    overhangThreshold_.first->SetValueRange(0, 90);
    overhangThreshold_.first->Step = 5;
    overhangThreshold_.first->Value = overhangThreshold_.second;

    controls_->Add(ControlTypeEnum::ksControlGroupEnd); /* printSettings */
}

double SettingsManager::getLayerHeight() {
    return layerHeight_.second;
}

double SettingsManager::getOverhangThreshold() {
    return overhangThreshold_.second;
}

bool SettingsManager::buttonClick(long buttonId) {
    /*
        Почему-то, при чтении Value, его значение запоминается в панели автоматически (если же это значение не прочитать, то оно не сохранится).
        Поэтому заненсение значений настроек в поля при показе панели и не реализовано (это происходит само по себе).
    */
    switch (buttonId) {
    case SpecPropertyButtonEnum::pbEnter:
        layerHeight_.second = layerHeight_.first->Value;
        overhangThreshold_.second = overhangThreshold_.first->Value;

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

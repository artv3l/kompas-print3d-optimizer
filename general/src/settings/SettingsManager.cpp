#include "stdafx.h"
#include "settings/SettingsManager.hpp"

#include <utility>
#include <unordered_map>
#include <stdexcept>

#include "apiutil/PropertyManagerObject.hpp"
#include "settings/DocumentsManager.hpp"
#include "settings/DocumentData.hpp"

SettingsManager::SettingsManager(KompasObjectPtr kompas, DocumentsManager& documentsManager):
    PropertyManagerObject(kompas),
    m_documentsManager(documentsManager),
    m_mainTab(propertyManager_->PropertyTabs->Add("MainTab")), m_controls(m_mainTab->PropertyControls),
    m_editMap(),
    m_shownSettings(nullptr)
{
    propertyManager_->Layout = PropertyManagerLayout::pmAlignRight;
    propertyManager_->SpecToolbar = SpecPropertyToolBarEnum::pnEnterEscHelp;
    propertyManager_->Caption = _T("Параметры печати");

    initControls();
}

void SettingsManager::show(DocumentData::Settings& settings) {
    m_shownSettings = &settings;
    settings.refreshVariables();
    fillSettingsToEdits(settings);
    PropertyManagerObject::show();
}

bool SettingsManager::buttonClick(long buttonId) {
        switch (buttonId) {
    case SpecPropertyButtonEnum::pbEnter:
        for (std::pair<std::string, IPropertyEditPtr> kv : m_editMap) {
            NumericSetting::Ptr setting = m_shownSettings->getSetting(kv.first);
            setting->setValue(kv.second->Value);
        }
        m_shownSettings->refreshVariables();
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

void SettingsManager::createEdit(SettingInitializer settingInitializer, ControlTypeEnum type, _bstr_t editName) {
    IPropertyEditPtr edit = m_editMap.insert(std::make_pair(settingInitializer.variableName, m_controls->Add(type))).first->second;
    edit->Name = editName;
    edit->SetValueRange(settingInitializer.range.first, settingInitializer.range.second);
    edit->Step = settingInitializer.step;
    edit->Value = settingInitializer.defaultValue;
}

void SettingsManager::initControls() {
    {
        IPropertyGroupBeginPtr printSettingsGroupBegin = m_controls->Add(ControlTypeEnum::ksControlGroupBegin);
        printSettingsGroupBegin->Name = "Параметры печати";
        printSettingsGroupBegin->Expanding = true;

        createEdit(SI_LAYER_HEIGHT, ControlTypeEnum::ksControlEditReal, "Высота слоя");
        createEdit(SI_OVERHANG_THRESHOLD, ControlTypeEnum::ksControlEditInt, "Максимальный угол нависаний");

        m_controls->Add(ControlTypeEnum::ksControlGroupEnd); /* printSettings */
    }
    {
        IPropertyGroupBeginPtr roundingGroupBegin = m_controls->Add(ControlTypeEnum::ksControlGroupBegin);
        roundingGroupBegin->Name = "Выпирающие углы";
        roundingGroupBegin->Expanding = true;

        createEdit(SI_ROUNDING_RADIUS, ControlTypeEnum::ksControlEditReal, "Радиус");
        createEdit(SI_ROUNDING_DEFLECTION_ANGLE, ControlTypeEnum::ksControlEditInt, "Угол отклонения");

        m_controls->Add(ControlTypeEnum::ksControlGroupEnd); /* rounding */
    }
    {
        IPropertyGroupBeginPtr elephantFootGroupBegin = m_controls->Add(ControlTypeEnum::ksControlGroupBegin);
        elephantFootGroupBegin->Name = "Слоновья нога";
        elephantFootGroupBegin->Expanding = true;

        createEdit(SI_ELEPHANT_FOOT_LAYERS_COUNT, ControlTypeEnum::ksControlEditInt, "Кол-во слоев");

        m_controls->Add(ControlTypeEnum::ksControlGroupEnd); /* elephantFoot */
    }
    {
        IPropertyGroupBeginPtr bridgeHoleFillGroupBegin = m_controls->Add(ControlTypeEnum::ksControlGroupBegin);
        bridgeHoleFillGroupBegin->Name = "Нависающие отверстия: закрытие диафрагмой";
        bridgeHoleFillGroupBegin->Expanding = true;

        createEdit(SI_BRIDGE_HOLE_FILL_LAYERS_COUNT, ControlTypeEnum::ksControlEditInt, "Слоев в диафрагме");

        m_controls->Add(ControlTypeEnum::ksControlGroupEnd); /* bridgeHoleFill */
    }
    {
        IPropertyGroupBeginPtr bridgeHoleBuildGroupBegin = m_controls->Add(ControlTypeEnum::ksControlGroupBegin);
        bridgeHoleBuildGroupBegin->Name = "Нависающие отверстия: достройка до набора мостов";
        bridgeHoleBuildGroupBegin->Expanding = true;

        createEdit(SI_BRIDGE_HOLE_BUILD_LAYERS_COUNT, ControlTypeEnum::ksControlEditInt, "Слоев в мосте");

        m_controls->Add(ControlTypeEnum::ksControlGroupEnd); /* bridgeHoleBuild */
    }
}

void SettingsManager::fillSettingsToEdits(DocumentData::Settings& settings) {
    for (std::pair<std::string, IPropertyEditPtr> kv : m_editMap) {
        NumericSetting::Ptr setting = settings.getSetting(kv.first);
        double val = setting->getValue();
        kv.second->Value = setting->getValue();
    }
}

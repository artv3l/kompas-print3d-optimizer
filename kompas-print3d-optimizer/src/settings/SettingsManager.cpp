#include "SettingsManager.hpp"

#include <utility>
#include <unordered_map>
#include <memory>

#include "kapiwrap/PropertyManagerObject.hpp"
#include "settings/Settings.hpp"
#include "global.hpp"

SettingsManager::SettingsManager(kapi::KompasObjectPtr kompas):
    PropertyManagerObject(kompas),
    m_mainTab(m_propertyManager->PropertyTabs->Add("MainTab")), m_controls(m_mainTab->PropertyControls),
    m_editMap(),
    m_shownSettings(nullptr)
{
    m_propertyManager->Layout = kapi::PropertyManagerLayout::pmAlignRight;
    m_propertyManager->SpecToolbar = kapi::SpecPropertyToolBarEnum::pnEnterEscHelp;
    m_propertyManager->Caption = _T("Параметры печати");

    initControls();
}

void SettingsManager::show(Settings* settings) {
    m_shownSettings = settings;
    fillSettingsToEdits();
    PropertyManagerObject::show();
}

bool SettingsManager::buttonClick(long buttonId) {
    switch (buttonId) {
    case kapi::SpecPropertyButtonEnum::pbEnter:
        for (std::pair<std::string, kapi::IPropertyEditPtr> kv : m_editMap) {
            Setting::Ptr setting = m_shownSettings->getSetting(kv.first);
            setting->setVariantValue(kv.second->Value);
        }
        m_shownSettings->uploadToDocument(global::kompas->ActiveDocument3D());
        hide();
        break;
    case kapi::SpecPropertyButtonEnum::pbHelp:
        break;
    case kapi::SpecPropertyButtonEnum::pbEsc:
        hide();
        break;
    }
    return true;
}

void SettingsManager::createEdit(const DoubleSettingInitializer& settingInitializer, kapi::ControlTypeEnum type, _bstr_t editName) {
    kapi::IPropertyEditPtr edit = m_controls->Add(type);
    m_editMap.insert(std::make_pair(settingInitializer.name, edit));
    edit->Name = editName;
    edit->SetValueRange(settingInitializer.range.first, settingInitializer.range.second);
    edit->Step = settingInitializer.step;
    edit->Value = settingInitializer.defaultValue;
}

void SettingsManager::createEdit(const StringSettingInitializer& settingInitializer, _bstr_t editName) {
    kapi::IPropertyEditPtr edit = m_controls->Add(kapi::ControlTypeEnum::ksControlEditStr);
    m_editMap.insert(std::make_pair(settingInitializer.name, edit));
    edit->Name = editName;
    edit->Value = settingInitializer.defaultValue;
}

void SettingsManager::initControls() {
    {
        kapi::IPropertyGroupBeginPtr printSettingsGroupBegin = m_controls->Add(kapi::ControlTypeEnum::ksControlGroupBegin);
        printSettingsGroupBegin->Name = "Параметры печати";
        printSettingsGroupBegin->Expanding = true;

        createEdit(si::layerHeight, kapi::ControlTypeEnum::ksControlEditReal, "Высота слоя");
        createEdit(si::overhangThreshold, kapi::ControlTypeEnum::ksControlEditInt, "Максимальный угол нависаний");

        m_controls->Add(kapi::ControlTypeEnum::ksControlGroupEnd); /* printSettings */
    }
    {
        kapi::IPropertyGroupBeginPtr roundingGroupBegin = m_controls->Add(kapi::ControlTypeEnum::ksControlGroupBegin);
        roundingGroupBegin->Name = "Выпирающие углы";
        roundingGroupBegin->Expanding = true;

        createEdit(si::roundingRadius, kapi::ControlTypeEnum::ksControlEditReal, "Радиус");
        createEdit(si::roundingDeflectionAngle, kapi::ControlTypeEnum::ksControlEditInt, "Угол отклонения");

        m_controls->Add(kapi::ControlTypeEnum::ksControlGroupEnd); /* rounding */
    }
    {
        kapi::IPropertyGroupBeginPtr elephantFootGroupBegin = m_controls->Add(kapi::ControlTypeEnum::ksControlGroupBegin);
        elephantFootGroupBegin->Name = "Слоновья нога";
        elephantFootGroupBegin->Expanding = true;

        createEdit(si::elephantFootLayersCount, kapi::ControlTypeEnum::ksControlEditInt, "Кол-во слоев");

        m_controls->Add(kapi::ControlTypeEnum::ksControlGroupEnd); /* elephantFoot */
    }
    {
        kapi::IPropertyGroupBeginPtr bridgeHoleFillGroupBegin = m_controls->Add(kapi::ControlTypeEnum::ksControlGroupBegin);
        bridgeHoleFillGroupBegin->Name = "Нависающие отверстия: закрытие диафрагмой";
        bridgeHoleFillGroupBegin->Expanding = true;

        createEdit(si::bridgeHoleFillLayersCount, kapi::ControlTypeEnum::ksControlEditInt, "Слоев в диафрагме");

        m_controls->Add(kapi::ControlTypeEnum::ksControlGroupEnd); /* bridgeHoleFill */
    }
    {
        kapi::IPropertyGroupBeginPtr bridgeHoleBuildGroupBegin = m_controls->Add(kapi::ControlTypeEnum::ksControlGroupBegin);
        bridgeHoleBuildGroupBegin->Name = "Нависающие отверстия: достройка до набора мостов";
        bridgeHoleBuildGroupBegin->Expanding = true;

        createEdit(si::bridgeHoleBuildLayersCount, kapi::ControlTypeEnum::ksControlEditInt, "Слоев в мосте");

        m_controls->Add(kapi::ControlTypeEnum::ksControlGroupEnd); /* bridgeHoleBuild */
    }
    {
        kapi::IPropertyGroupBeginPtr exportGroupBegin = m_controls->Add(kapi::ControlTypeEnum::ksControlGroupBegin);
        exportGroupBegin->Name = "Экспорт";
        exportGroupBegin->Expanding = true;

        createEdit(si::exportStlFolder, "Папка для stl");

        m_controls->Add(kapi::ControlTypeEnum::ksControlGroupEnd); /* export */
    }
}

void SettingsManager::fillSettingsToEdits() {
    for (std::pair<std::string, kapi::IPropertyEditPtr> kv : m_editMap) {
        Setting::Ptr setting = m_shownSettings->getSetting(kv.first);
        kv.second->Value = setting->getVariantValue();
    }
}

#include "SettingsManager.hpp"

#include <utility>
#include <unordered_map>
#include <memory>

#include "kapiwrap/PropertyManagerObject.hpp"
#include "settings/Settings.hpp"

SettingsManager::SettingsManager(KompasObjectPtr kompas):
    PropertyManagerObject(kompas),
    m_mainTab(m_propertyManager->PropertyTabs->Add("MainTab")), m_controls(m_mainTab->PropertyControls),
    m_editMap(),
    m_shownSettings(nullptr)
{
    m_propertyManager->Layout = PropertyManagerLayout::pmAlignRight;
    m_propertyManager->SpecToolbar = SpecPropertyToolBarEnum::pnEnterEscHelp;
    m_propertyManager->Caption = _T("Параметры печати");

    initControls();
}

void SettingsManager::show(Settings& settings) {
    m_shownSettings = &settings;
    settings.loadFromDocument();
    fillSettingsToEdits(settings);
    PropertyManagerObject::show();
}

bool SettingsManager::buttonClick(long buttonId) {
    switch (buttonId) {
    case SpecPropertyButtonEnum::pbEnter:
        for (std::pair<std::string, IPropertyEditPtr> kv : m_editMap) {
            Setting::Ptr setting = m_shownSettings->getSetting(kv.first);
            setting->setVariantValue(kv.second->Value);
        }
        m_shownSettings->uploadToDocument();
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

void SettingsManager::createEdit(NumericSettingInitializer settingInitializer, ControlTypeEnum type, _bstr_t editName) {
    IPropertyEditPtr edit = m_controls->Add(type);
    m_editMap.insert(std::make_pair(settingInitializer.name, edit));
    edit->Name = editName;
    edit->SetValueRange(settingInitializer.range.first, settingInitializer.range.second);
    edit->Step = settingInitializer.step;
    edit->Value = settingInitializer.defaultValue;
}

void SettingsManager::createEdit(StringSettingInitializer settingInitializer, _bstr_t editName) {
    IPropertyEditPtr edit = m_controls->Add(ControlTypeEnum::ksControlEditStr);
    m_editMap.insert(std::make_pair(settingInitializer.name, edit));
    edit->Name = editName;
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
    {
        IPropertyGroupBeginPtr exportGroupBegin = m_controls->Add(ControlTypeEnum::ksControlGroupBegin);
        exportGroupBegin->Name = "Экспорт";
        exportGroupBegin->Expanding = true;

        createEdit(SI_EXPORT_STL_FOLDER, "Папка для stl");

        m_controls->Add(ControlTypeEnum::ksControlGroupEnd); /* export */
    }
}

void SettingsManager::fillSettingsToEdits(Settings& settings) {
    for (std::pair<std::string, IPropertyEditPtr> kv : m_editMap) {
        Setting::Ptr setting = m_shownSettings->getSetting(kv.first);
        kv.second->Value = setting->getVariantValue();
    }
}

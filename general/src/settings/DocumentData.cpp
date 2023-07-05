#include "stdafx.h"
#include "settings/DocumentData.hpp"

#include <unordered_map>
#include <memory>
#include <utility>

#include "settings/Setting.hpp"
#include "settings/SettingInitializer.hpp"
#include "apiutil/Macro.hpp"
#include "Optional.hpp"

const char* DocumentData::ROOT_MACRO_NAME = "Оптимизации";

DocumentData::DocumentData(ksDocument3DPtr document3d):
    m_part(document3d->GetPart(pTop_Part)), m_settings(document3d), m_rootMacro(getOrCreateRootMacro(m_part))
{}

DocumentData::Settings& DocumentData::getSettings() {
    return m_settings;
}

Macro DocumentData::getRootMacro() {
    if (!m_rootMacro.isCreated() || (m_rootMacro.getName() != _bstr_t(ROOT_MACRO_NAME))) {
        m_rootMacro = Macro(m_part, ROOT_MACRO_NAME, true);
    }
    return m_rootMacro;
}

Macro DocumentData::getOrCreateRootMacro(ksPartPtr part) {
    ksEntityPtr macroEntity = Macro::findMacro(part, ROOT_MACRO_NAME);
    if (!macroEntity) {
        return Macro(part, ROOT_MACRO_NAME, true);
    } else {
        return Macro(macroEntity);
    }
}

DocumentData::Settings::Settings(ksDocument3DPtr document3d) :
    m_document3d(document3d), m_variableCollection(nullptr), m_printSurface(), m_SettingsMap()
{
    ksPartPtr part(document3d->GetPart(pTop_Part));
    ksFeaturePtr feature(part->GetFeature());
    m_variableCollection = feature->VariableCollection;

    for (NumericSettingInitializer settingInitializer : VARIABLE_SETTING_INITIALIZERS) {
        m_SettingsMap.insert(std::make_pair(
            settingInitializer.variableName,
            std::make_shared<VariableNumericSetting>(VariableNumericSetting(part, settingInitializer))
        ));
    }
    for (NumericSettingInitializer settingInitializer : LOCAL_SETTING_INITIALIZERS) {
        m_SettingsMap.insert(std::make_pair(
            settingInitializer.variableName,
            std::make_shared<LocalNumericSetting>(LocalNumericSetting(settingInitializer))
        ));
    }
    for (StringSettingInitializer settingInitializer : STRING_SETTING_INITIALIZERS) {
        m_SettingsMap.insert(std::make_pair(
            settingInitializer.variableName,
            std::make_shared<StringSetting>(StringSetting(settingInitializer))
        ));
    }
}

void DocumentData::Settings::refreshVariables() const {
    m_variableCollection->refresh();
    m_document3d->RebuildDocument();
}

void DocumentData::Settings::setPrintSurface(PrintSurface printSurface) {
    m_printSurface = printSurface;
}

bool DocumentData::Settings::isPrintSurfaceSelected() const {
    return static_cast<bool>(m_printSurface);
}

PrintSurface DocumentData::Settings::getPrintSurface() const {
    if (!m_printSurface) {
        throw std::runtime_error("Плоскость печати не выбрана");
    }
    return m_printSurface.value();
}

Setting::Ptr DocumentData::Settings::getSetting(std::string name) {
    return m_SettingsMap[name];
}

NumericSetting::Ptr DocumentData::Settings::getNumericSetting(std::string name) {
    Setting::Ptr setting = getSetting(name);
    NumericSetting::Ptr numericSetting = std::dynamic_pointer_cast<NumericSetting>(setting);
    if (!numericSetting) {
        throw std::runtime_error("There is no NumericSetting with name \"" + name + "\"");
    }
    return numericSetting;
}

StringSetting::Ptr DocumentData::Settings::getStringSetting(std::string name) {
    Setting::Ptr setting = getSetting(name);
    StringSetting::Ptr stringSetting = std::dynamic_pointer_cast<StringSetting>(setting);
    if (!stringSetting) {
        throw std::runtime_error("There is no StringSetting with name \"" + name + "\"");
    }
    return stringSetting;
}

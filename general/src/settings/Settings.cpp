#include "stdafx.h"
#include "settings/Settings.hpp"

#include <memory>
#include <utility>
#include <unordered_map>
#include <string>
#include <stdexcept>

#include "settings/SettingInitializer.hpp"
#include "settings/Setting.hpp"
#include "Optional.hpp"
#include "settings/PrintSurface.hpp"

Settings::Settings(ksDocument3DPtr document3d) :
    m_document3d(document3d), m_variableCollection(nullptr), m_printSurface(), m_settingsMap() {
    ksPartPtr part(document3d->GetPart(pTop_Part));
    ksFeaturePtr feature(part->GetFeature());
    m_variableCollection = feature->VariableCollection;

    for (std::pair<std::string, NumericSettingInitializer> pair : VARIABLE_SETTING_INITIALIZERS) {
        m_settingsMap.insert(std::make_pair(
            pair.first,
            std::make_shared<VariableNumericSetting>(VariableNumericSetting(part, pair.second))
        ));
    }
    for (std::pair<std::string, NumericSettingInitializer> pair : LOCAL_SETTING_INITIALIZERS) {
        m_settingsMap.insert(std::make_pair(
            pair.first,
            std::make_shared<LocalNumericSetting>(LocalNumericSetting(pair.second))
        ));
    }
    for (std::pair<std::string, StringSettingInitializer> pair : STRING_SETTING_INITIALIZERS) {
        m_settingsMap.insert(std::make_pair(
            pair.first,
            std::make_shared<StringSetting>(StringSetting(pair.second))
        ));
    }
}

void Settings::loadFromDocument() {
    m_variableCollection->refresh();
    for (SettingsMap::iterator it = m_settingsMap.begin(); it != m_settingsMap.end(); it++) {
        VariableNumericSetting::Ptr vns = std::dynamic_pointer_cast<VariableNumericSetting>(it->second);
        if (vns) {
            vns->loadOrCreateVariable(m_document3d->GetPart(Part_Type::pTop_Part), vns->getName(),
                                      VARIABLE_SETTING_INITIALIZERS.at(vns->getName()).note,
                                      VARIABLE_SETTING_INITIALIZERS.at(vns->getName()).defaultValue);
        }
    }
}

void Settings::uploadToDocument() {
    for (SettingsMap::iterator it = m_settingsMap.begin(); it != m_settingsMap.end(); it++) {
        VariableNumericSetting::Ptr vns = std::dynamic_pointer_cast<VariableNumericSetting>(it->second);
        if (vns) {
            vns->createIfNotExists(m_document3d->GetPart(Part_Type::pTop_Part),
                                   VARIABLE_SETTING_INITIALIZERS.at(vns->getName()).note,
                                   VARIABLE_SETTING_INITIALIZERS.at(vns->getName()).defaultValue);
        }
    }
    m_variableCollection->refresh();
    m_document3d->RebuildDocument();
}

void Settings::setPrintSurface(PrintSurface printSurface) {
    m_printSurface = printSurface;
}

bool Settings::isPrintSurfaceSelected() const {
    return static_cast<bool>(m_printSurface);
}

PrintSurface Settings::getPrintSurface() const {
    if (!m_printSurface) {
        throw std::runtime_error("Плоскость печати не выбрана");
    }
    return m_printSurface.value();
}

Setting::Ptr Settings::getSetting(std::string name) {
    return m_settingsMap[name];
}

NumericSetting::Ptr Settings::getNumericSetting(std::string name) {
    Setting::Ptr setting = getSetting(name);
    NumericSetting::Ptr numericSetting = std::dynamic_pointer_cast<NumericSetting>(setting);
    if (!numericSetting) {
        throw std::runtime_error("There is no NumericSetting with name \"" + name + "\"");
    }
    return numericSetting;
}

StringSetting::Ptr Settings::getStringSetting(std::string name) {
    Setting::Ptr setting = getSetting(name);
    StringSetting::Ptr stringSetting = std::dynamic_pointer_cast<StringSetting>(setting);
    if (!stringSetting) {
        throw std::runtime_error("There is no StringSetting with name \"" + name + "\"");
    }
    return stringSetting;
}

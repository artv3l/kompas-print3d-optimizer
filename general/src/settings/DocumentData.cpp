#include "stdafx.h"
#include "settings/DocumentData.hpp"

#include <unordered_map>
#include <memory>
#include <utility>

#include "settings/NumericSetting.hpp"
#include "settings/SettingInitializer.hpp"

const char* DocumentData::ROOT_MACRO_NAME = "Оптимизации";

DocumentData::DocumentData(ksDocument3DPtr document3d):
    m_settings(document3d), m_rootMacro(Macro(document3d->GetPart(pTop_Part), ROOT_MACRO_NAME, true))
{}

DocumentData::Settings& DocumentData::getSettings() {
    return m_settings;
}

Macro DocumentData::getRootMacro() const {
    return m_rootMacro;
}

DocumentData::Settings::Settings(ksDocument3DPtr document3d) :
    m_document3d(document3d), m_variableCollection(nullptr), m_printSurface(), m_numericSettings()
{
    ksPartPtr part(document3d->GetPart(pTop_Part));
    ksFeaturePtr feature(part->GetFeature());
    m_variableCollection = feature->VariableCollection;

    for (SettingInitializer settingInitializer : VARIABLE_SETTING_INITIALIZERS) {
        m_numericSettings.insert(std::make_pair(
            settingInitializer.variableName,
            std::make_shared<VariableNumericSetting>(VariableNumericSetting(part, settingInitializer))
        ));
    }
    for (SettingInitializer settingInitializer : LOCAL_SETTING_INITIALIZERS) {
        m_numericSettings.insert(std::make_pair(
            settingInitializer.variableName,
            std::make_shared<LocalNumericSetting>(LocalNumericSetting(settingInitializer))
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

NumericSetting::Ptr DocumentData::Settings::getSetting(std::string name) {
    return m_numericSettings[name];
}

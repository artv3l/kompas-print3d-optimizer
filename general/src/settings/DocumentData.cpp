#include "stdafx.h"
#include "settings/DocumentData.hpp"

#include <unordered_map>
#include <memory>
#include <utility>

#include "settings/Setting.hpp"
#include "settings/SettingInitializer.hpp"
#include "apiutil/Macro.hpp"
#include "Optional.hpp"
#include "FrameEventImpl.hpp"

const char* DocumentData::ROOT_MACRO_NAME = "Оптимизации";

DocumentData::DocumentData(KompasObjectPtr kompas, ksDocument3DPtr document3d):
    m_part(document3d->GetPart(pTop_Part)), m_settings(document3d), m_rootMacro(), m_frameEvent()
{
    IKompasDocumentPtr document7 = kompas->TransferInterface(document3d, ksAPITypeEnum::ksAPI7Dual, 0);
    IDocumentFramesPtr documentFrames = document7->DocumentFrames;
    IDocumentFramePtr documentFrame = documentFrames->GetItem(0);
    m_frameEvent = new FrameEventImpl(documentFrame);
}

DocumentData::Settings& DocumentData::getSettings() {
    return m_settings;
}

Macro DocumentData::getOrCreateRootMacro() {
    if (!m_rootMacro || !m_rootMacro.value().isCreated() || (m_rootMacro.value().getName() != _bstr_t(ROOT_MACRO_NAME))) {
        ksEntityPtr macroEntity = Macro::findMacro(m_part, ROOT_MACRO_NAME);
        if (!macroEntity) {
            m_rootMacro = Macro(m_part, ROOT_MACRO_NAME, true);
        } else {
            m_rootMacro = Macro(macroEntity);
        }
    }
    return m_rootMacro.value();
}

DocumentData::Settings::Settings(ksDocument3DPtr document3d) :
    m_document3d(document3d), m_variableCollection(nullptr), m_printSurface(), m_settingsMap()
{
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

void DocumentData::Settings::loadFromDocument() {
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

void DocumentData::Settings::uploadToDocument() {
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
    return m_settingsMap[name];
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

#include "Settings.hpp"

#include <memory>
#include <utility>
#include <unordered_map>
#include <string>
#include <stdexcept>

#include "settings/SettingInitializer.hpp"
#include "settings/Setting.hpp"
#include "settings/PrintSurface.hpp"

Settings::Settings() :
    m_printSurface(),
    m_settingsMap()
{
    for (si::SettingInitializerMap::value_type pair : si::settingInitializers) {
        m_settingsMap.insert(
            std::make_pair(pair.first, pair.second->create())
        );
    }
}

void Settings::setPrintSurface(PrintSurface printSurface) {
    m_printSurface = printSurface;
}

bool Settings::isPrintSurfaceSelected() const {
    return static_cast<bool>(m_printSurface);
}

std::optional<PrintSurface> Settings::getPrintSurface() const {
    return m_printSurface;
}

Setting::Ptr Settings::getSetting(std::string name) {
    return m_settingsMap[name];
}

DoubleSetting::Ptr Settings::getDoubleSetting(std::string name) {
    Setting::Ptr setting = getSetting(name);
    DoubleSetting::Ptr doubleSetting = std::dynamic_pointer_cast<DoubleSetting>(setting);
    if (!doubleSetting) {
        throw std::runtime_error("There is no DoubleSetting with name \"" + name + "\"");
    }
    return doubleSetting;
}

StringSetting::Ptr Settings::getStringSetting(std::string name) {
    Setting::Ptr setting = getSetting(name);
    StringSetting::Ptr stringSetting = std::dynamic_pointer_cast<StringSetting>(setting);
    if (!stringSetting) {
        throw std::runtime_error("There is no StringSetting with name \"" + name + "\"");
    }
    return stringSetting;
}

void Settings::loadFromDocument(kapi::ksDocument3DPtr document3d) {
    kapi::ksPartPtr part(document3d->GetPart(kapi::pTop_Part));
    kapi::ksFeaturePtr feature(part->GetFeature());
    kapi::ksVariableCollectionPtr variableCollection(feature->VariableCollection);
    variableCollection->refresh();

    for (SettingsMap::iterator it = m_settingsMap.begin(); it != m_settingsMap.end(); it++) {
        if (!it->second->isSyncWithDocument()) {
            continue;
        }
        
        _bstr_t variableName = (c_variableNamePrefix + it->second->getName()).c_str();

        kapi::ksVariablePtr variable = variableCollection->GetByName(variableName, true, false);
        if (variable) {
            // Только DoubleSetting может синхронизироваться с документом
            DoubleSetting::Ptr doubleSetting = std::static_pointer_cast<DoubleSetting>(it->second);
            doubleSetting->setValue(variable->value);
        }
    }
}

void Settings::uploadToDocument(kapi::ksDocument3DPtr document3d) {
    kapi::ksPartPtr part(document3d->GetPart(kapi::pTop_Part));
    kapi::ksFeaturePtr feature(part->GetFeature());
    kapi::ksVariableCollectionPtr variableCollection(feature->VariableCollection);

    for (SettingsMap::iterator it = m_settingsMap.begin(); it != m_settingsMap.end(); it++) {
        if (!it->second->isSyncWithDocument()) {
            continue;
        }

        // Только DoubleSetting может синхронизироваться с документом
        DoubleSetting::Ptr doubleSetting = std::static_pointer_cast<DoubleSetting>(it->second);

        _bstr_t variableName = (c_variableNamePrefix + it->second->getName()).c_str();
        kapi::ksVariablePtr variable = variableCollection->GetByName(variableName, true, false);
        if (!variable) {
            const DoubleSettingInitializer* dsi = static_cast<const DoubleSettingInitializer*>(si::settingInitializers.at(it->second->getName()));
            variable = variableCollection->AddNewVariable(variableName, doubleSetting->getValue(), dsi->note);
        } else {
            variable->value = doubleSetting->getValue();
        }

    }
    variableCollection->refresh();
    document3d->RebuildDocument();
}

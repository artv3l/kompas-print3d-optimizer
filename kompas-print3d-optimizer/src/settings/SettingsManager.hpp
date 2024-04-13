#ifndef SETTINGS_MANAGER_HPP
#define SETTINGS_MANAGER_HPP

#include <unordered_map>
#include <string>
#include <comutil.h>

#include "kapiwrap/PropertyManagerObject.hpp"
#include "settings/SettingInitializer.hpp"

class Settings;

class SettingsManager : public PropertyManagerObject {
public:
    SettingsManager(kapi::KompasObjectPtr kompas);
    virtual ~SettingsManager() = default;

    void show(Settings* settings);

private:
    using EditMap = std::unordered_map<std::string, kapi::IPropertyEditPtr>;

    kapi::IPropertyTabPtr m_mainTab;
    kapi::IPropertyControlsPtr m_controls;
    EditMap m_editMap;
    Settings* m_shownSettings;
    
    virtual bool buttonClick(long buttonId) override;

    void createEdit(const DoubleSettingInitializer& settingInitializer, kapi::ControlTypeEnum type, _bstr_t editName);
    void createEdit(const StringSettingInitializer& settingInitializer, _bstr_t editName);

    void initControls();
    void fillSettingsToEdits();
};

#endif /* SETTINGS_MANAGER_HPP */

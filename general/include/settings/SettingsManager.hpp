#ifndef SETTINGS_MANAGER_HPP
#define SETTINGS_MANAGER_HPP

#include <unordered_map>
#include <string>
#include <comutil.h>

#include "apiutil/PropertyManagerObject.hpp"
#include "SettingInitializer.hpp"

class Settings;

class SettingsManager : public PropertyManagerObject {
public:
    SettingsManager(KompasObjectPtr kompas);
    virtual ~SettingsManager() = default;

    void show(Settings& settings);

private:
    using EditMap = std::unordered_map<std::string, IPropertyEditPtr>;

    IPropertyTabPtr m_mainTab;
    IPropertyControlsPtr m_controls;
    EditMap m_editMap;
    Settings* m_shownSettings;
    
    virtual bool buttonClick(long buttonId) override;

    void createEdit(NumericSettingInitializer settingInitializer, ControlTypeEnum type, _bstr_t editName);
    void createEdit(StringSettingInitializer settingInitializer, _bstr_t editName);

    void initControls();
    void fillSettingsToEdits(Settings& settings);
};

#endif /* SETTINGS_MANAGER_HPP */

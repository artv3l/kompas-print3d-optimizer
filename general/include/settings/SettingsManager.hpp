#ifndef SETTINGS_MANAGER_HPP
#define SETTINGS_MANAGER_HPP

#include <utility>
#include <unordered_map>
#include <memory>
#include <string>
#include <comutil.h>

#include "apiutil/PropertyManagerObject.hpp"
#include "PrintSurface.hpp"
#include "Optional.hpp"
#include "DocumentsManager.hpp"
#include "DocumentData.hpp"
#include "SettingInitializer.hpp"

class SettingsManager : public PropertyManagerObject {
public:
    SettingsManager(KompasObjectPtr kompas, DocumentsManager& documentsManager);
    virtual ~SettingsManager() = default;

    void show(DocumentData::Settings& settings);

private:
    using EditMap = std::unordered_map<std::string, IPropertyEditPtr>;

    DocumentsManager& m_documentsManager;
    IPropertyTabPtr m_mainTab;
    IPropertyControlsPtr m_controls;
    EditMap m_editMap;
    DocumentData::Settings* m_shownSettings;
    
    virtual bool buttonClick(long buttonId) override;

    void createEdit(NumericSettingInitializer settingInitializer, ControlTypeEnum type, _bstr_t editName);
    void createEdit(StringSettingInitializer settingInitializer, _bstr_t editName);

    void initControls();
    void fillSettingsToEdits(DocumentData::Settings& settings);
};

#endif /* SETTINGS_MANAGER_HPP */

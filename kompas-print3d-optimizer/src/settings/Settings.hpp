#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <string>
#include <memory>

#include "settings/PrintSurface.hpp"
#include "settings/Setting.hpp"
#include "Optional.hpp"

class Settings {
public:
    Settings(ksDocument3DPtr document3d);

    void loadFromDocument();
    void uploadToDocument();
    void setPrintSurface(PrintSurface printSurface);
    bool isPrintSurfaceSelected() const;
    PrintSurface getPrintSurface() const;
    Setting::Ptr getSetting(std::string name);
    NumericSetting::Ptr getNumericSetting(std::string name);
    StringSetting::Ptr getStringSetting(std::string name);

private:
    using SettingsMap = std::unordered_map<std::string, Setting::Ptr>;

    ksDocument3DPtr m_document3d;
    ksVariableCollectionPtr m_variableCollection;
    Optional<PrintSurface> m_printSurface;
    SettingsMap m_settingsMap;
};


#endif /* SETTINGS_HPP */

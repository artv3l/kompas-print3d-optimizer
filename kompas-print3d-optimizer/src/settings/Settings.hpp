#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <string>
#include <memory>
#include <unordered_map>

#include "settings/PrintSurface.hpp"
#include "settings/Setting.hpp"
#include "Optional.hpp"

class Settings {
public:
    const std::string c_variableNamePrefix = "kp3do_";

    Settings();
    Settings(const Settings& obj) = delete;
    Settings(Settings&& obj) noexcept = delete;
    Settings& operator=(const Settings& obj) = delete;
    Settings& operator=(Settings&& obj) noexcept = delete;
    ~Settings() = default;

    void setPrintSurface(PrintSurface printSurface);
    bool isPrintSurfaceSelected() const;
    PrintSurface getPrintSurface() const;
    Setting::Ptr getSetting(std::string name);
    DoubleSetting::Ptr getDoubleSetting(std::string name);
    StringSetting::Ptr getStringSetting(std::string name);

    void loadFromDocument(ksDocument3DPtr document3d);
    void uploadToDocument(ksDocument3DPtr document3d);

private:
    using SettingsMap = std::unordered_map<std::string, Setting::Ptr>;

    Optional<PrintSurface> m_printSurface;
    SettingsMap m_settingsMap;
};


#endif /* SETTINGS_HPP */

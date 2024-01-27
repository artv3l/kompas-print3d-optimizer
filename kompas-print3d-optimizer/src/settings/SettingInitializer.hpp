#ifndef SETTING_INITIALIZER_HPP
#define SETTING_INITIALIZER_HPP

#include <string>
#include <utility>
#include <comutil.h>
#include <unordered_map>

#include "Setting.hpp"

struct SettingInitializer {
    const std::string name;

    SettingInitializer(std::string name_);

    virtual Setting::Ptr create() const = 0;
};

struct DoubleSettingInitializer : public SettingInitializer {
    const bool isSyncWithDocument;
    const double defaultValue;
    const std::pair<double, double> range;
    const double step;
    const _bstr_t note;

    DoubleSettingInitializer(const std::string& name_, bool isSyncWithDocument_, double defaultValue_,
                             std::pair<double, double> range_, double step_, const _bstr_t& note_);

    virtual Setting::Ptr create() const override;
};

struct StringSettingInitializer : public SettingInitializer {
    const _bstr_t defaultValue;

    StringSettingInitializer(const std::string& name_, const _bstr_t& defaultValue_);

    virtual Setting::Ptr create() const override;
};

namespace setting_initializer {
    using SettingInitializerMap = std::unordered_map<std::string, const SettingInitializer*>;

    extern const DoubleSettingInitializer layerHeight;
    extern const DoubleSettingInitializer overhangThreshold;
    extern const DoubleSettingInitializer roundingRadius;
    extern const DoubleSettingInitializer roundingDeflectionAngle;
    extern const DoubleSettingInitializer elephantFootLayersCount;
    extern const DoubleSettingInitializer bridgeHoleFillLayersCount;
    extern const DoubleSettingInitializer bridgeHoleBuildLayersCount;

    extern const StringSettingInitializer exportStlFolder;

    extern const SettingInitializerMap settingInitializers;
}

namespace si = setting_initializer;

#endif /* SETTING_INITIALIZER_HPP */

#ifndef SETTING_INITIALIZER_HPP
#define SETTING_INITIALIZER_HPP

#include <string>
#include <utility>
#include <comutil.h>
#include <list>

struct SettingInitializer {
    const std::string variableName;
    const double defaultValue;
    const std::pair<double, double> range;
    const double step;
    _bstr_t variableNote;
};

extern const SettingInitializer SI_LAYER_HEIGHT;
extern const SettingInitializer SI_OVERHANG_THRESHOLD;
extern const SettingInitializer SI_ROUNDING_RADIUS;
extern const SettingInitializer SI_ROUNDING_DEFLECTION_ANGLE;
extern const SettingInitializer SI_ELEPHANT_FOOT_LAYERS_COUNT;
extern const SettingInitializer SI_BRIDGE_HOLE_FILL_LAYERS_COUNT;
extern const SettingInitializer SI_BRIDGE_HOLE_BUILD_LAYERS_COUNT;

extern const std::list<SettingInitializer> VARIABLE_SETTING_INITIALIZERS;
extern const std::list<SettingInitializer> LOCAL_SETTING_INITIALIZERS;

#endif /* SETTING_INITIALIZER_HPP */

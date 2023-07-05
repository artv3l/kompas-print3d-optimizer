#ifndef SETTING_INITIALIZER_HPP
#define SETTING_INITIALIZER_HPP

#include <string>
#include <utility>
#include <comutil.h>
#include <list>

struct NumericSettingInitializer {
    const std::string variableName;
    const double defaultValue;
    const std::pair<double, double> range;
    const double step;
    _bstr_t variableNote;
};

struct StringSettingInitializer {
    const std::string variableName;
    const _bstr_t defaultValue;
};

extern const NumericSettingInitializer SI_LAYER_HEIGHT;
extern const NumericSettingInitializer SI_OVERHANG_THRESHOLD;
extern const NumericSettingInitializer SI_ROUNDING_RADIUS;
extern const NumericSettingInitializer SI_ROUNDING_DEFLECTION_ANGLE;
extern const NumericSettingInitializer SI_ELEPHANT_FOOT_LAYERS_COUNT;
extern const NumericSettingInitializer SI_BRIDGE_HOLE_FILL_LAYERS_COUNT;
extern const NumericSettingInitializer SI_BRIDGE_HOLE_BUILD_LAYERS_COUNT;

extern const StringSettingInitializer SI_EXPORT_STL_FOLDER;

extern const std::list<NumericSettingInitializer> VARIABLE_SETTING_INITIALIZERS;
extern const std::list<NumericSettingInitializer> LOCAL_SETTING_INITIALIZERS;
extern const std::list<StringSettingInitializer> STRING_SETTING_INITIALIZERS;

#endif /* SETTING_INITIALIZER_HPP */

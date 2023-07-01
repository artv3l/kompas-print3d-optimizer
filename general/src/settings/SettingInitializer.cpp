#include "settings/SettingInitializer.hpp"

#include <utility>
#include <list>

const SettingInitializer SI_LAYER_HEIGHT {"layer_height", 0.2, std::make_pair(0.04, 0.4), 0.04, "Высота слоя"};
const SettingInitializer SI_OVERHANG_THRESHOLD {"overhang_threshold", 45.0, std::make_pair(0.0, 90.0), 5.0, "Максимальный угол нависаний"};
const SettingInitializer SI_ROUNDING_RADIUS {"rounding_radius", 1.0, std::make_pair(0.1, 10.0), 0.2, ""};
const SettingInitializer SI_ROUNDING_DEFLECTION_ANGLE {"rounding_deflection_angle", 5.0, std::make_pair(0.0, 20.0), 1.0, ""};
const SettingInitializer SI_ELEPHANT_FOOT_LAYERS_COUNT {"elephant_foot_layers_count", 2.0, std::make_pair(1.0, 5.0), 1.0, ""};
const SettingInitializer SI_BRIDGE_HOLE_FILL_LAYERS_COUNT {"bridge_hole_fill_layers_count", 1.0, std::make_pair(1.0, 5.0), 1.0, ""};
const SettingInitializer SI_BRIDGE_HOLE_BUILD_LAYERS_COUNT{"bridge_hole_build_layers_count", 1.0, std::make_pair(1.0, 5.0), 1.0, ""};

const std::list<SettingInitializer> VARIABLE_SETTING_INITIALIZERS {
    SI_LAYER_HEIGHT, SI_OVERHANG_THRESHOLD,
};

const std::list<SettingInitializer> LOCAL_SETTING_INITIALIZERS {
    SI_ROUNDING_RADIUS, SI_ROUNDING_DEFLECTION_ANGLE,
    SI_ELEPHANT_FOOT_LAYERS_COUNT,
    SI_BRIDGE_HOLE_FILL_LAYERS_COUNT, SI_BRIDGE_HOLE_BUILD_LAYERS_COUNT,
};

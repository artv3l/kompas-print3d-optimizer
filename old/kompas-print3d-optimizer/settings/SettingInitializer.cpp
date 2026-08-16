#include "SettingInitializer.hpp"

#include <utility>
#include <unordered_map>
#include <memory>

#include "Setting.hpp"

namespace setting_initializer {
    const DoubleSettingInitializer layerHeight { "layer_height", true, 0.2, std::make_pair(0.04, 0.4), 0.04, "Высота слоя" };
    const DoubleSettingInitializer overhangThreshold { "overhang_threshold", true, 45.0, std::make_pair(0.0, 90.0), 5.0, "Максимальный угол нависаний" };
    const DoubleSettingInitializer roundingRadius { "rounding_radius", false, 1.0, std::make_pair(0.1, 10.0), 0.2, "" };
    const DoubleSettingInitializer roundingDeflectionAngle { "rounding_deflection_angle", false, 5.0, std::make_pair(0.0, 20.0), 1.0, "" };
    const DoubleSettingInitializer elephantFootLayersCount { "elephant_foot_layers_count", false, 2.0, std::make_pair(1.0, 5.0), 1.0, "" };
    const DoubleSettingInitializer bridgeHoleFillLayersCount { "bridge_hole_fill_layers_count", false, 1.0, std::make_pair(1.0, 5.0), 1.0, "" };
    const DoubleSettingInitializer bridgeHoleBuildLayersCount { "bridge_hole_build_layers_count", false, 1.0, std::make_pair(1.0, 5.0), 1.0, "" };

    const StringSettingInitializer exportStlFolder { "export_stl_folder", "stl" };

    const SettingInitializerMap settingInitializers {
        {layerHeight.name, &layerHeight},
        {overhangThreshold.name, &overhangThreshold},
        {roundingRadius.name, &roundingRadius},
        {roundingDeflectionAngle.name, &roundingDeflectionAngle},
        {elephantFootLayersCount.name, &elephantFootLayersCount},
        {bridgeHoleFillLayersCount.name, &bridgeHoleFillLayersCount},
        {bridgeHoleBuildLayersCount.name, &bridgeHoleBuildLayersCount},
        {exportStlFolder.name, &exportStlFolder},
    };
}

SettingInitializer::SettingInitializer(std::string name_) :
    name(name_) {}

DoubleSettingInitializer::DoubleSettingInitializer(const std::string& name_, bool isSyncWithDocument_, double defaultValue_,
                                                   std::pair<double, double> range_, double step_, const _bstr_t& note_):
    SettingInitializer(name_), isSyncWithDocument(isSyncWithDocument_),
    defaultValue(defaultValue_), range(range_), step(step_), note(note_)
{}

Setting::Ptr DoubleSettingInitializer::create() const {
    return std::make_shared<DoubleSetting>(name, isSyncWithDocument, defaultValue);
}

StringSettingInitializer::StringSettingInitializer(const std::string& name_, const _bstr_t& defaultValue_):
    SettingInitializer(name_), defaultValue(defaultValue_)
{}

Setting::Ptr StringSettingInitializer::create() const {
    return std::make_shared<StringSetting>(name, defaultValue);
}

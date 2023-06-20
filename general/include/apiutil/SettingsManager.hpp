#ifndef SETTINGS_MANAGER_HPP
#define SETTINGS_MANAGER_HPP

#include "stdafx.h"

#include <utility>

#include "PropertyManagerObject.hpp"

struct Settings {
    double layerHeight;
    double overhangThreshold;

    double roundingRadius;
    double roundingDeflectionAngle;

    uint8_t elephantFootLayersCount;

    uint8_t bridgeHoleFillLayersCount;
    uint8_t bridgeHoleBuildLayersCount;
};

class SettingsManager : public PropertyManagerObject {
public:
    SettingsManager(KompasObjectPtr kompas);
    virtual ~SettingsManager() = default;

    Settings getSettings() const;

private:
    virtual bool buttonClick(long buttonId) override;

    Settings settings_;

    IPropertyTabPtr mainTab_;
    IPropertyControlsPtr controls_;

    IPropertyEditPtr layerHeightEdit_;
    IPropertyEditPtr overhangThresholdEdit_;

    IPropertyEditPtr roundingRadiusEdit_;
    IPropertyEditPtr roundingDeflectionAngleEdit_;

    IPropertyEditPtr elephantFootLayersCountEdit_;

    IPropertyEditPtr bridgeHoleFillLayersCountEdit_;
    IPropertyEditPtr bridgeHoleBuildLayersCountEdit_;
};

#endif /* SETTINGS_MANAGER_HPP */

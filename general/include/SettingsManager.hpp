#ifndef SETTINGS_MANAGER_HPP
#define SETTINGS_MANAGER_HPP

#include "stdafx.h"

#include <utility>

#include "PropertyManagerObject.hpp"

class SettingsManager : public PropertyManagerObject {
public:
    SettingsManager(KompasObjectPtr kompas);
    virtual ~SettingsManager() = default;

    double getLayerHeight();
    double getOverhangThreshold();

private:
    virtual bool buttonClick(long buttonId) override;

    IPropertyTabPtr mainTab_;
    IPropertyControlsPtr controls_;

    std::pair<IPropertyEditPtr, double> layerHeight_;
    std::pair<IPropertyEditPtr, int> overhangThreshold_;

};


#endif /* SETTINGS_MANAGER_HPP */

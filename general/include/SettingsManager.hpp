#ifndef SETTINGS_MANAGER_HPP
#define SETTINGS_MANAGER_HPP

#include "stdafx.h"
#include "PropertyManagerObject.hpp"

class SettingsManager : public PropertyManagerObject {
public:
    SettingsManager(KompasObjectPtr kompas);
    virtual ~SettingsManager() = default;

private:
    virtual bool buttonClick(long buttonId) override;

};


#endif /* SETTINGS_MANAGER_HPP */

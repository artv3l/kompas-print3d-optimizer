#ifndef PROPERTY_MANAGER_EVENT_HPP
#define PROPERTY_MANAGER_EVENT_HPP

#include "stdafx.h"

#include <functional>

#include "AutomationBaseEvent.hpp"

class PropertyManagerObject;

class PropertyManagerEvent : public AutomationBaseEvent {
public:
    PropertyManagerEvent(PropertyManagerObject *propertyManager);
    virtual ~PropertyManagerEvent() = default;

    afx_msg bool buttonClick(long buttonId);

    DECLARE_EVENTSINK_MAP();

private:
    PropertyManagerObject *m_propertyManager;
    
};

#endif /* PROPERTY_MANAGER_EVENT_HPP */

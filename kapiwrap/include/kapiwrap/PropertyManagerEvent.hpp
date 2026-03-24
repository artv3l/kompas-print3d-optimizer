#ifndef PROPERTY_MANAGER_EVENT_HPP
#define PROPERTY_MANAGER_EVENT_HPP

#include "AutomationBaseEvent.hpp"

class PropertyManagerObject;

class PropertyManagerEvent : public AutomationBaseEvent {
public:
    PropertyManagerEvent(PropertyManagerObject* propertyManager);
    virtual ~PropertyManagerEvent() = default;

    afx_msg bool buttonClick(long buttonId);
    afx_msg bool changeControlValue(IDispatch* control);
    afx_msg bool controlCommand(IDispatch* control, long buttonId);
    afx_msg bool buttonUpdate(long buttonId, long* check, short* enable);
    afx_msg bool processActivate();
    afx_msg bool processDeactivate();
    afx_msg bool commandHelp(long buttonId);
    afx_msg bool selectItem(IDispatch* control, long index, bool select);

    DECLARE_EVENTSINK_MAP();

private:
    PropertyManagerObject* m_propertyManager;
    
};

#endif /* PROPERTY_MANAGER_EVENT_HPP */

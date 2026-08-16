#include "kapiwrap/PropertyManagerEvent.hpp"

#include <afxdisp.h>

#include "kapiwrap/AutomationBaseEvent.hpp"
#include "kapiwrap/PropertyManagerObject.hpp"

PropertyManagerEvent::PropertyManagerEvent(PropertyManagerObject *propertyManager) :
        AutomationBaseEvent(static_cast<IUnknown *>(propertyManager->getPropertyManager()), kapi::DIID_ksPropertyManagerNotify),
        m_propertyManager(propertyManager)
{
    advise();
}

// Карта сообщений
BEGIN_EVENTSINK_MAP(PropertyManagerEvent, AutomationBaseEvent)
    ON_EVENT(PropertyManagerEvent, (unsigned int)-1, kapi::ksPropertyManagerNotifyEnum::prButtonClick, PropertyManagerEvent::buttonClick, VTS_I4)
    ON_EVENT(PropertyManagerEvent, (unsigned int)-1, kapi::ksPropertyManagerNotifyEnum::prChangeControlValue, PropertyManagerEvent::changeControlValue, VTS_DISPATCH)
    ON_EVENT(PropertyManagerEvent, (unsigned int)-1, kapi::ksPropertyManagerNotifyEnum::prControlCommand, PropertyManagerEvent::controlCommand, VTS_DISPATCH VTS_I4)
    ON_EVENT(PropertyManagerEvent, (unsigned int)-1, kapi::ksPropertyManagerNotifyEnum::prButtonUpdate, PropertyManagerEvent::buttonUpdate, VTS_I4 VTS_PI4 VTS_PBOOL)
    ON_EVENT(PropertyManagerEvent, (unsigned int)-1, kapi::ksPropertyManagerNotifyEnum::prProcessActivate, PropertyManagerEvent::processActivate, VTS_NONE)
    ON_EVENT(PropertyManagerEvent, (unsigned int)-1, kapi::ksPropertyManagerNotifyEnum::prProcessDeactivate, PropertyManagerEvent::processDeactivate, VTS_NONE)
    ON_EVENT(PropertyManagerEvent, (unsigned int)-1, kapi::ksPropertyManagerNotifyEnum::prCommandHelp, PropertyManagerEvent::commandHelp, VTS_I4)
    ON_EVENT(PropertyManagerEvent, (unsigned int)-1, kapi::ksPropertyManagerNotifyEnum::prSelectItem, PropertyManagerEvent::selectItem, VTS_DISPATCH VTS_I4 VTS_BOOL)
END_EVENTSINK_MAP()

afx_msg bool PropertyManagerEvent::buttonClick(long buttonId) {
    return m_propertyManager->buttonClick(buttonId);
}

bool PropertyManagerEvent::changeControlValue(IDispatch* control) {
    return m_propertyManager->changeControlValue(control);
}

bool PropertyManagerEvent::controlCommand(IDispatch* control, long buttonId) {
    return m_propertyManager->controlCommand(control, buttonId);
}

bool PropertyManagerEvent::buttonUpdate(long buttonId, long* check, short* enable) {
    return m_propertyManager->buttonUpdate(buttonId, check, enable);
}

bool PropertyManagerEvent::processActivate() {
    return m_propertyManager->processActivate();
}

bool PropertyManagerEvent::processDeactivate() {
    return m_propertyManager->processDeactivate();
}

bool PropertyManagerEvent::commandHelp(long buttonId) {
    return m_propertyManager->commandHelp(buttonId);
}

bool PropertyManagerEvent::selectItem(IDispatch* control, long index, bool select) {
    return m_propertyManager->selectItem(control, index, select);
}

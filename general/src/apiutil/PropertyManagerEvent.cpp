#include "stdafx.h"
#include "apiutil/PropertyManagerEvent.hpp"

#include <afxdisp.h>

#include "apiutil/AutomationBaseEvent.hpp"
#include "apiutil/PropertyManagerObject.hpp"

PropertyManagerEvent::PropertyManagerEvent(PropertyManagerObject *propertyManager) :
        AutomationBaseEvent(static_cast<IUnknown *>(propertyManager->getPropertyManager()), DIID_ksPropertyManagerNotify),
        m_propertyManager(propertyManager) {
    advise();
}

// Карта сообщений
BEGIN_EVENTSINK_MAP(PropertyManagerEvent, AutomationBaseEvent)
    ON_EVENT(PropertyManagerEvent, (unsigned int)-1, ksPropertyManagerNotifyEnum::prButtonClick, PropertyManagerEvent::buttonClick, VTS_I4)
END_EVENTSINK_MAP()

afx_msg bool PropertyManagerEvent::buttonClick(long buttonId) {
    return m_propertyManager->buttonClick(buttonId);
}

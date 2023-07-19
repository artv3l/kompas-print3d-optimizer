#include "stdafx.h"
#include "apiutil/DocumentFrameEvent.hpp"

#include <afxdisp.h>

#include "apiutil/AutomationBaseEvent.hpp"

DocumentFrameEvent::DocumentFrameEvent(IDocumentFramePtr documentFrame) :
    AutomationBaseEvent(static_cast<IUnknown*>(documentFrame), DIID_ksDocumentFrameNotify)
{
    advise();
}

// Карта сообщений
BEGIN_EVENTSINK_MAP(DocumentFrameEvent, AutomationBaseEvent)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frActivate, DocumentFrameEvent::activate, VTS_NONE)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frCloseFrame, DocumentFrameEvent::closeFrame, VTS_NONE)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frClosePaintGL, DocumentFrameEvent::closePaintGL, VTS_DISPATCH VTS_I4)
END_EVENTSINK_MAP()

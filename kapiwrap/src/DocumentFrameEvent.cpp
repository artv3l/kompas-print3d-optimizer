#include "DocumentFrameEvent.hpp"

#include <afxdisp.h>

#include "AutomationBaseEvent.hpp"

DocumentFrameEvent::DocumentFrameEvent(IDocumentFramePtr documentFrame) :
    AutomationBaseEvent(static_cast<IUnknown*>(documentFrame), DIID_ksDocumentFrameNotify),
    m_documentFrame(documentFrame)
{
    advise();
}

// Карта сообщений
BEGIN_EVENTSINK_MAP(DocumentFrameEvent, AutomationBaseEvent)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frActivate, DocumentFrameEvent::activate, VTS_NONE)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frCloseFrame, DocumentFrameEvent::closeFrame, VTS_NONE)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frClosePaintGL, DocumentFrameEvent::closePaintGL, VTS_DISPATCH VTS_I4)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frDeactivate, DocumentFrameEvent::deactivate, VTS_NONE)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frMouseDown, DocumentFrameEvent::mouseDown, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frMouseMove, DocumentFrameEvent::mouseMove, VTS_I2 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

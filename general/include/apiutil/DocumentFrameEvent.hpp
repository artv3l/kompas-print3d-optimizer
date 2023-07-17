#ifndef DOCUMENT_FRAME_EVENT_HPP
#define DOCUMENT_FRAME_EVENT_HPP

#include "apiutil/AutomationBaseEvent.hpp"

class DocumentFrameEvent : public AutomationBaseEvent {
public:
    DocumentFrameEvent(IDocumentFramePtr documentFrame);
    virtual ~DocumentFrameEvent() = default;

    afx_msg bool closePaintGL(ksGLObject* glObject, long drawMode);

    DECLARE_EVENTSINK_MAP();
};

#endif /* DOCUMENT_FRAME_EVENT_HPP */

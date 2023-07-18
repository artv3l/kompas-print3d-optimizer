#ifndef DOCUMENT_FRAME_EVENT_HPP
#define DOCUMENT_FRAME_EVENT_HPP

#include "apiutil/AutomationBaseEvent.hpp"

class DocumentFrameEvent : public AutomationBaseEvent {
public:
    DocumentFrameEvent(IDocumentFramePtr documentFrame);
    virtual ~DocumentFrameEvent() = default;

    virtual afx_msg bool activate() = 0;
    virtual afx_msg bool closePaintGL(ksGLObject* glObject, long drawMode) = 0;

private:
    DECLARE_EVENTSINK_MAP();
};

#endif /* DOCUMENT_FRAME_EVENT_HPP */

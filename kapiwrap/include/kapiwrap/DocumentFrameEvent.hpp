#ifndef DOCUMENT_FRAME_EVENT_HPP
#define DOCUMENT_FRAME_EVENT_HPP

#include <afxwin.h>

#include "AutomationBaseEvent.hpp"

class DocumentFrameEvent : public AutomationBaseEvent {
public:
    DocumentFrameEvent(kapi::IDocumentFramePtr documentFrame);
    virtual ~DocumentFrameEvent() = default;

protected:
    kapi::IDocumentFramePtr m_documentFrame;

    virtual afx_msg bool activate() = 0;
    virtual afx_msg bool closeFrame() = 0;
    virtual afx_msg bool closePaintGL(kapi::ksGLObject* glObject, long drawMode) = 0;
    virtual afx_msg bool deactivate() = 0;
    virtual afx_msg bool mouseDown(short nButton, short nShiftState, long x, long y) = 0;
    virtual afx_msg bool mouseMove(short nShiftState, long x, long y) = 0;

private:
    DECLARE_EVENTSINK_MAP();
};

#endif /* DOCUMENT_FRAME_EVENT_HPP */

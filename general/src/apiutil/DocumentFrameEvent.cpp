#include "stdafx.h"
#include "apiutil/DocumentFrameEvent.hpp"

#include <iostream>

#include "glad/glad.h"

DocumentFrameEvent::DocumentFrameEvent(IDocumentFramePtr documentFrame) :
    AutomationBaseEvent(static_cast<IUnknown*>(documentFrame), DIID_ksDocumentFrameNotify)
{
    advise();
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
}

bool DocumentFrameEvent::mouseDown(short nButton, short nShiftState, long x, long y) {
    std::cout << "x=" << x << "; y=" << y << "\n";
    return false;
}

bool DocumentFrameEvent::closePaintGL(ksGLObject* glObject, long drawMode) {
    std::cout << "closePaintGL" << "\n";
    return false;
}

// Карта сообщений
BEGIN_EVENTSINK_MAP(DocumentFrameEvent, AutomationBaseEvent)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frMouseDown, DocumentFrameEvent::mouseDown, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frClosePaintGL, DocumentFrameEvent::closePaintGL, VTS_DISPATCH VTS_I4)
END_EVENTSINK_MAP()

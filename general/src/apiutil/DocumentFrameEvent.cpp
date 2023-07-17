#include "stdafx.h"
#include "apiutil/DocumentFrameEvent.hpp"

#include <iostream>

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

DocumentFrameEvent::DocumentFrameEvent(IDocumentFramePtr documentFrame) :
    AutomationBaseEvent(static_cast<IUnknown*>(documentFrame), DIID_ksDocumentFrameNotify)
{
    advise();
}

bool DocumentFrameEvent::closePaintGL(ksGLObject* glObject, long drawMode) {
    std::cout << "closePaintGL" << "\n";
    
    float matrix4[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix4);
    glm::mat4 modelview = glm::make_mat4(matrix4);
    glGetFloatv(GL_PROJECTION_MATRIX, matrix4);
    glm::mat4 projection = glm::make_mat4(matrix4);

    return false;
}

// Карта сообщений
BEGIN_EVENTSINK_MAP(DocumentFrameEvent, AutomationBaseEvent)
    ON_EVENT(DocumentFrameEvent, (unsigned int)-1, ksDocumentFrameNotifyEnum::frClosePaintGL, DocumentFrameEvent::closePaintGL, VTS_DISPATCH VTS_I4)
END_EVENTSINK_MAP()

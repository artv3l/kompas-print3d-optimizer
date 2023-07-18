#ifndef FRAME_EVENT_IMPL_HPP
#define FRAME_EVENT_IMPL_HPP

#include <memory>

#include "apiutil/DocumentFrameEvent.hpp"
#include "glutil/Shader.hpp"

class FrameEventImpl : public DocumentFrameEvent {
public:
    FrameEventImpl(IDocumentFramePtr documentFrame);
    virtual ~FrameEventImpl() = default;

    bool activate() override;
    bool closePaintGL(ksGLObject* glObject, long drawMode) override;
private:
    static ShaderProgram::Ptr s_shaderProgram;

    static void initShaders();


};

#endif /* FRAME_EVENT_IMPL_HPP */

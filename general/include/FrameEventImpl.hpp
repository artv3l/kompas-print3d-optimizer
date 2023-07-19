#ifndef FRAME_EVENT_IMPL_HPP
#define FRAME_EVENT_IMPL_HPP

#include <memory>

#include "glutil/Shader.hpp"
#include "apiutil/DocumentFrameEvent.hpp"

class Settings;

class FrameEventImpl : public DocumentFrameEvent {
public:
    FrameEventImpl(KompasObjectPtr kompas, ksDocument3DPtr document3d, Settings* settings);
    virtual ~FrameEventImpl() = default;

    bool activate() override;
    bool closeFrame() override;
    bool closePaintGL(ksGLObject* glObject, long drawMode) override;

private:
    static ShaderProgram::Ptr s_shaderProgram;

    ksDocument3DPtr m_document3d;
    Settings* m_settings;

    static void initShaders();
    static IDocumentFramePtr getDocumentFrame(KompasObjectPtr kompas, ksDocument3DPtr document3d);
};

#endif /* FRAME_EVENT_IMPL_HPP */

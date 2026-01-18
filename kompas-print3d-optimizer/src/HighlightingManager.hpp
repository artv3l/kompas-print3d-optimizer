#ifndef HIGHLIGHTING_MANAGER_HPP
#define HIGHLIGHTING_MANAGER_HPP

#include <memory>

#include "oglwrap/Shader.hpp"
#include "kapiwrap/DocumentFrameEvent.hpp"

class Settings;

class HighlightingManager : public DocumentFrameEvent {
public:
    enum Mode : uint8_t {
        off = 0x00, layersEverywhere = 0x01, layersAtCursor = 0x02, overhangs = 0x04,
    };

    HighlightingManager(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, Settings* settings);
    virtual ~HighlightingManager() = default;

    void toggleMode(Mode mode);
    void refreshWindow() const;

private:
    kapi::ksDocument3DPtr m_document3d;
    Settings* m_settings;
    uint8_t m_mode;
    glm::vec2 m_mouseCoord;

private: /* static */
    static std::unique_ptr<ShaderProgram> s_shaderProgram;
    static bool s_isGladInited;
    static short s_framesCount;
    
    static void initShaders();
    static kapi::IDocumentFramePtr getDocumentFrame(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d);
    static void drawTriangulation(kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace);
    
private: /* events */
    bool activate() override;
    bool closeFrame() override;
    bool beginPaintGL(kapi::ksGLObject* glObject, long drawMode) override;
    bool closePaintGL(kapi::ksGLObject* glObject, long drawMode) override;
    bool deactivate() override;
    bool mouseDown(short nButton, short nShiftState, long x, long y) override;
    bool mouseMove(short nShiftState, long x, long y) override;

};

#endif /* HIGHLIGHTING_MANAGER_HPP */

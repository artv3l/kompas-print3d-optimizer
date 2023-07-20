#ifndef HIGHLIGHTING_MANAGER_HPP
#define HIGHLIGHTING_MANAGER_HPP

#include <memory>
#include <bitset>

#include "glutil/Shader.hpp"
#include "apiutil/DocumentFrameEvent.hpp"

class Settings;

class HighlightingManager : public DocumentFrameEvent {
public:
    enum class Mode {
        layersEverywhere, layersAtCursor, overhangs,
    };

    HighlightingManager(KompasObjectPtr kompas, ksDocument3DPtr document3d, Settings* settings);
    virtual ~HighlightingManager() = default;

    void toggleMode(Mode mode);

private:
    ksDocument3DPtr m_document3d;
    Settings* m_settings;
    std::bitset<3> m_mode;

private: /* static */
    static ShaderProgram::Ptr s_shaderProgram;
    static bool s_isGladInited;
    static short s_framesCount;
    
    static void initShaders();
    static IDocumentFramePtr getDocumentFrame(KompasObjectPtr kompas, ksDocument3DPtr document3d);
    static void drawTriangulation(ksPartPtr part);
    
private: /* events */
    bool activate() override;
    bool closeFrame() override;
    bool closePaintGL(ksGLObject* glObject, long drawMode) override;
    bool deactivate() override;
    bool mouseDown(short nButton, short nShiftState, long x, long y) override;

};

#endif /* HIGHLIGHTING_MANAGER_HPP */

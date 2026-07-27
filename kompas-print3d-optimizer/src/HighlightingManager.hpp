#ifndef HIGHLIGHTING_MANAGER_HPP
#define HIGHLIGHTING_MANAGER_HPP

#include <memory>

#include "oglwrap/Shader.hpp"
#include "oglwrap/VertexArray.hpp"
#include "kapiwrap/DocumentFrameEvent.hpp"
#include "mesh.hpp"
#include "settings/PrintSurface.hpp"
#include "drawing/DrawingManager.hpp"

class Settings;

class HighlightingManager : public DocumentFrameEvent {
public:
    enum Mode : uint8_t {
        off = 0x00,
        layersEverywhere = 0x01, layersAtCursor = 0x02,
        overhangs = 0x04,
    };

    HighlightingManager(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, Settings* settings);
    virtual ~HighlightingManager() = default;

    void toggleMode(Mode mode);
    void refreshWindow() const;

    void addObject(std::shared_ptr<IObject> object, Visualizer visualizer);
    void cleanObjects();

private:
    kapi::ksDocument3DPtr m_document3d;
    Settings* m_settings;
    uint8_t m_mode;
    glm::vec2 m_mouseCoord;

    std::unordered_map<Visualizer, std::vector<DrawableMesh>> m_objects;

private: /* static */
    static std::unordered_map<Visualizer, ShaderProgram> m_shaders;
    static bool s_isGladInited;
    static short s_framesCount;
    
    static void initShaders();
    static kapi::IDocumentFramePtr getDocumentFrame(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d);
    
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

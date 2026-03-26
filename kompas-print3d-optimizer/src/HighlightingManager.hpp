#ifndef HIGHLIGHTING_MANAGER_HPP
#define HIGHLIGHTING_MANAGER_HPP

#include <memory>

#include "oglwrap/Shader.hpp"
#include "oglwrap/VertexArray.hpp"
#include "kapiwrap/DocumentFrameEvent.hpp"
#include "mesh.hpp"
#include "settings/PrintSurface.hpp"

class Settings;

class DrawableMesh
{
public:
    DrawableMesh(std::shared_ptr<geometry::IObject> object, std::shared_ptr<ShaderProgram> shaderProgram);
    virtual ~DrawableMesh() = default;
    void draw() const;
    std::shared_ptr<ShaderProgram> m_shaderProgram;
protected:
    VertexArray m_vao;
    size_t m_count; // РљРѕР»-РІРѕ РёРЅРґРµРєСЃРѕРІ РґР»СЏ РѕС‚СЂРёСЃРѕРІРєРё
    int m_mode;
};

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

    void addObject(std::shared_ptr<geometry::IObject> object);
    void cleanObjects();

private:
    kapi::ksDocument3DPtr m_document3d;
    Settings* m_settings;
    uint8_t m_mode;
    glm::vec2 m_mouseCoord;

    std::vector<DrawableMesh> m_objects;

private: /* static */
    static std::shared_ptr<ShaderProgram> s_shaderProgram;
    static std::shared_ptr<ShaderProgram> s_shaderOrientationEvalMesh;
    static std::shared_ptr<ShaderProgram> s_shaderPolyline;
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

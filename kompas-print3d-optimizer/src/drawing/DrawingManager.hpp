#pragma once

#include <memory>

#include <KsAPI.h>

#include "oglwrap/Mesh.hpp"
#include "oglwrap/VertexArray.hpp"
#include "oglwrap/Shader.hpp"
#include "oglwrap/DrawableMesh.hpp"

// Р РµР¶РёРј РІРёР·СѓР°Р»РёР·Р°С†РёРё РјРѕРґРµР»Рё. РџРѕ СЃСѓС‚Рё С€РµР№РґРµСЂРЅР°СЏ РїСЂРѕРіСЂР°РјРјР°
enum class Visualizer : uint8_t
{
    meshHighlight3dp, // РЎРµС‚РєР° СЃ РїРѕРґСЃРІРµС‚РєР°РјРё РґР»СЏ 3D РїРµС‡Р°С‚Рё
    colorMesh,        // Р¦РІРµС‚РЅР°СЏ СЃРµС‚РєР°
    smoothMesh,       // РЎРµС‚РєР° СЃ С†РІРµС‚Р°РјРё РїРѕ РІРµСЂС€РёРЅР°Рј
    polyline,         // РџРѕР»РёР»РёРЅРёСЏ
};

class DrawingManager final
{
public:
	DrawingManager(ksapi::IDocumentFramePtr frame, std::wstring_view eventsOwnerName);
    ~DrawingManager();

    void addObject(std::shared_ptr<IObject> object, Visualizer visualizer);
    void cleanObjects();
    void redraw();
    void setBackgroundColor(const color::RGB& backgroundColor);

private:
    void close();
    bool beginPaintGL(uint32_t drawMode, const ksapi::IOpenGLObjectPtr& glObject);
    void closePaintGL(uint32_t drawMode, const ksapi::IOpenGLObjectPtr& glObject);
    bool mouseMove(const ksapi::IPressedKeysPtr& pressedKeys, int32_t x, int32_t y);

private:
	ksapi::IDocumentFramePtr m_frame;
    ksapi::IDocumentFrameEventsPtr m_frameEvents;
    const std::wstring m_eventsOwnerName;

    color::RGB m_backgroundColor;
    std::unordered_map<Visualizer, std::vector<std::unique_ptr<IDrawableObject>>> m_objects;

private: // deprecated?
    static std::unordered_map<Visualizer, ShaderProgram> m_shaders;
    static bool s_isGladInited;
    static short s_framesCount;

    static void initShaders();
};

#include "DrawingManager.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders.hpp"

std::unordered_map<Visualizer, ShaderProgram> DrawingManager::m_shaders;
bool DrawingManager::s_isGladInited = false;
short DrawingManager::s_framesCount = 0;

namespace
{
void* GetAnyGLFuncAddress(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void*)GetProcAddress(module, name);
    }
    return p;
}
}

DrawingManager::DrawingManager(ksapi::IDocumentFramePtr frame, std::wstring_view eventsOwnerName):
	m_frame(frame),
	m_frameEvents(m_frame->Events()),
	m_eventsOwnerName(eventsOwnerName)
{
	namespace stdph = std::placeholders;

	m_frameEvents->AddBeginPaintGlHandler(m_eventsOwnerName, std::bind(&DrawingManager::beginPaintGL, this, stdph::_1, stdph::_2));
	m_frameEvents->AddClosePaintGlHandler(m_eventsOwnerName, std::bind(&DrawingManager::closePaintGL, this, stdph::_1, stdph::_2));
	m_frameEvents->AddCloseHandler(m_eventsOwnerName, std::bind(&DrawingManager::close, this));
	m_frameEvents->AddMouseMoveHandler(m_eventsOwnerName, std::bind(&DrawingManager::mouseMove, this, stdph::_1, stdph::_2, stdph::_3));

    {
        // GLAD РЅСѓР¶РЅРѕ РёРЅРёС†РёР°Р»РёР·РёСЂРѕРІР°С‚СЊ РєРѕРіРґР° РѕС‚РєСЂС‹С‚ РґРѕРєСѓРјРµРЅС‚
        if (!s_isGladInited) {
            if (!gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress)) {
                //global::kompas->ksMessage("РћС€РёР±РєР° РёРЅРёС†РёР°Р»РёР·Р°С†РёРё GLAD");
            }
        }
        /*
          РџРѕСЃР»Рµ Р·Р°РєСЂС‹С‚РёСЏ РІСЃРµС… РґРѕРєСѓРјРµРЅС‚РѕРІ СЃРѕСЃС‚РѕСЏРЅРёРµ OpenGL СЃР±СЂР°СЃС‹РІР°РµС‚СЃСЏ.
          РџРѕСЌС‚РѕРјСѓ, РєРѕРіРґР° РїРѕСЃР»Рµ СЌС‚РѕРіРѕ РѕС‚РєСЂС‹РІР°РµС‚СЃСЏ РЅРѕРІС‹Р№ РґРѕРєСѓРјРµРЅС‚, РЅСѓР¶РЅРѕ Р·Р°РЅРѕРІРѕ СЃРєРѕРјРїРёР»РёСЂРѕРІР°С‚СЊ С€РµР№РґРµСЂС‹
        */
        if (s_framesCount == 0) {
            try {
                initShaders();
            }
            catch (const std::runtime_error& e) {
                //global::kompas->ksMessage("РћС€РёР±РєР° РєРѕРјРїРёР»СЏС†РёРё С€РµР№РґРµСЂРѕРІ");
                //std::cerr << e.what() << "\n";
            }
        }
        s_framesCount++;
    }
}

DrawingManager::~DrawingManager()
{
	m_frameEvents->RemoveAllHandlers(m_eventsOwnerName);
}

void DrawingManager::addObject(std::shared_ptr<IObject> object, Visualizer visualizer)
{
    m_objects[visualizer].emplace_back(createDrawableMesh(object));
}

void DrawingManager::cleanObjects()
{
    m_objects.clear();
}

void DrawingManager::redraw()
{
    m_frame->RefreshWindow();
}

void DrawingManager::close()
{
    s_framesCount--;
    if (s_framesCount == 0) {
        m_shaders.clear();
    }
}

bool DrawingManager::beginPaintGL(uint32_t drawMode, const ksapi::IOpenGLObjectPtr& glObject)
{
    /*
        РџРѕС‡РµРјСѓ-С‚Рѕ РїРѕРґ Debug РєР°СЃС‚РѕРјРЅР°СЏ РѕС‚СЂРёСЃРѕРІРєР° СЂР°Р±РѕС‚Р°РµС‚ С‚РѕР»СЊРєРѕ РІ beginPaintGL,
            Р° РїРѕРґ Release С‚РѕР»СЊРєРѕ РІ closePaintGL.
            РўР°Рє РїСЂРѕРёСЃС…РѕРґРёС‚ РїСЂРё Р·Р°РїСЂРµС‚Рµ СЃС‚Р°РЅРґР°СЂС‚РЅРѕР№ РѕС‚СЂРёСЃРѕРІРєРё (РІРѕР·РІСЂР°С‚ false РёР·
            beginPaintGL).
        РћСЃС‚Р°РІР»СЏСЋ РІРєР»СЋС‡РµРЅРЅРѕР№ СЃС‚Р°РЅРґР°СЂС‚РЅСѓСЋ РѕС‚СЂРёСЃРѕРІРєСѓ. Р”Р°Р»РµРµ РІ closePaintGL Р·Р°С‚РёСЂР°СЋ
            СЂРµР·СѓР»СЊС‚Р°С‚ СЃС‚Р°РЅРґР°СЂС‚РЅРѕР№ РѕС‚СЂРёСЃРѕРІРєРё Рё РІС‹РІРѕР¶Сѓ РєР°СЃС‚РѕРјРЅСѓСЋ. РљРѕРЅРµС‡РЅРѕ, СЌС‚Рѕ С…СѓР¶Рµ
            РїРѕ РїСЂРѕРёР·РІРѕРґРёС‚РµР»СЊРЅРѕСЃС‚Рё, РЅРѕ Р·Р°С‚Рѕ РµРґРёРЅРѕРѕР±СЂР°Р·РЅРѕ.
    */
    return true;
}

void DrawingManager::closePaintGL(uint32_t drawMode, const ksapi::IOpenGLObjectPtr& glObject)
{
    if (m_objects.empty()) {
        return;
    }

    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    float matrix4[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix4);
    glm::mat4 modelview(glm::make_mat4(matrix4));
    glGetFloatv(GL_PROJECTION_MATRIX, matrix4);
    glm::mat4 projection(glm::make_mat4(matrix4));

    for (const auto& [visualizer, objects] : m_objects) {
        auto& shaderProgram = m_shaders.at(visualizer);

        auto lock = shaderProgram.use();
        shaderProgram.setUniform("u_modelview", modelview);
        shaderProgram.setUniform("u_projection", projection);

        /*
        enum Mode : uint8_t {
            off = 0x00,
            layersEverywhere = 0x01, layersAtCursor = 0x02,
            overhangs = 0x04,
        };

        void HighlightingManager::toggleMode(Mode mode) {
            m_mode ^= mode;
            if (mode & Mode::layersEverywhere) {
                m_mode &= ~Mode::layersAtCursor;
            } else if (mode & Mode::layersAtCursor) {
                m_mode &= ~Mode::layersEverywhere;
            }
        }
        */
#if 0
        if (visualizer == Visualizer::meshHighlight3dp) {
            if (!m_settings->isPrintSurfaceSelected())
                continue;

            /*
                Р’ СЃРїСЂР°РІРєРµ РЅР°РїРёСЃР°РЅРѕ, С‡С‚Рѕ РјРµС‚РѕРґ GetZoomScale СЂР°Р±РѕС‚Р°РµС‚ С‚РѕР»СЊРєРѕ РґР»СЏ РіСЂР°С„РёС‡РµСЃРєРёС… РґРѕРєСѓРјРµРЅС‚РѕРІ, РЅРѕ РґР»СЏ РјРѕРґРµР»Рё scale СЃС‡РёС‚Р°РµС‚СЃСЏ РєРѕСЂСЂРµРєС‚РЅРѕ.
                РџРѕСЌС‚РѕРјСѓ РµРіРѕ Рё Р±СѓРґРµРј РёСЃРїРѕР»СЊР·РѕРІР°С‚СЊ РґР»СЏ СЂР°СЃС‡РµС‚Р° С‚РѕР»С‰РёРЅС‹ Р»РёРЅРёРё РІ С€РµР№РґРµСЂРµ (РєР°РєР°СЏ РѕР±Р»Р°СЃС‚СЊ РІРѕРєСЂСѓРі С‚РѕС‡РЅРѕР№ РіСЂР°РЅРёС†С‹ СЃР»РѕРµРІ Р±СѓРґРµС‚ РѕС‚СЂРёСЃРѕРІС‹РІР°С‚СЊСЃСЏ).
                Р§РµРј Р±Р»РёР¶Рµ РјРѕРґРµР»СЊРєР°, С‚РµРј РјРµРЅСЊС€Рµ С‚РѕР»С‰РёРЅР° СЂРёСЃСѓРµРјРѕР№ Р»РёРЅРёРё.

                (scale, lineWidth): (410.2, 0.001), (137.4, 0.003), (31.9, 0.0085), (8.9, 0.025), (4.3, 0.033)
                Р°РїРїСЂРѕРєСЃРёРјРёСЂСѓРµРј СЃС‚РµРїРµРЅРЅРѕР№ С„СѓРЅРєС†РёРµР№: lineWidth = 0.1192 * scale^(-0.7731)

                РўР°РєР¶Рµ СЂР°СЃСЃС‡РёС‚Р°РµРј СЂР°РґРёСѓСЃ РѕРєСЂСѓР¶РЅРѕСЃС‚Рё, РІ РїСЂРµРґРµР»Р°С… РєРѕС‚РѕСЂРѕР№ Р±СѓРґСѓС‚ РѕС‚СЂРёСЃРѕРІС‹РІР°С‚СЊСЃСЏ СЃР»РѕРё РІ СЂРµР¶РёРјРµ РѕС‚СЂРёСЃРѕРІРєРё Сѓ РєСѓСЂСЃРѕСЂР°

                (scale, mouseRadius): (341.8, 450), (137.4, 220), (46.6, 110), (26.6, 90), (15.4, 60), (10.7, 70), (5.2, 40), (3.0, 20)
                Р°РїРїСЂРѕРєСЃРёРјРёСЂСѓРµРј СЃС‚РµРїРµРЅРЅРѕР№ С„СѓРЅРєС†РёРµР№: mouseRadius = 12.8668 * scale^(0.5943)
            */
            double unused, scale; m_documentFrame->GetZoomScale(&unused, &unused, &scale);
            float lineWidth = static_cast<float>(0.1192 * std::pow(scale, -0.7731));
            int mouseRadius = static_cast<int>(12.8668 * std::pow(scale, 0.5943));

            PrintSurface printSurface = *m_settings->getPrintSurface();
            glm::vec3 printSurfaceNormal(printSurface.eq.a, printSurface.eq.b, printSurface.eq.c);
            const double overhangThreshold = m_settings->getDoubleSetting(si::overhangThreshold.name)->getValue();

            kapi::ksPartPtr part = m_document3d->GetPart(kapi::Part_Type::pTop_Part);

            shaderProgram.setUniform("u_printSurfaceNormal", printSurfaceNormal);
            shaderProgram.setUniform("u_printSurfaceD", static_cast<float>(printSurface.eq.d));
            shaderProgram.setUniform("u_layerHeight", static_cast<float>(m_settings->getDoubleSetting(si::layerHeight.name)->getValue()));
            shaderProgram.setUniform("u_lineWidth", lineWidth);
            shaderProgram.setUniform("u_mode", m_mode);
            shaderProgram.setUniform("u_overhangThreshold", static_cast<float>(math::toRadians(overhangThreshold)));
            shaderProgram.setUniform("u_mouseCoord", m_mouseCoord);
            shaderProgram.setUniform("u_mouseRadius", mouseRadius);
        }
#endif

        for (auto& obj : objects) {
            obj->draw(shaderProgram);
        }
    }
}

bool DrawingManager::mouseMove(const ksapi::IPressedKeysPtr& pressedKeys, int32_t x, int32_t y)
{
    /*m_mouseCoord = glm::vec2(x, y);
    m_documentFrame->RefreshWindow();*/
	return false;
}

void DrawingManager::initShaders()
{
    /*m_shaders.emplace(std::piecewise_construct, std::forward_as_tuple(Visualizer::meshHighlight3dp),
        std::forward_as_tuple(VERTEX_SHADER_CODE, FRAGMENT_SHADER_CODE));*/
    m_shaders.emplace(std::piecewise_construct, std::forward_as_tuple(Visualizer::colorMesh),
        std::forward_as_tuple(VERTEX_SHADER_CODE_ORIENTATION, FRAGMENT_SHADER_CODE_ORIENTATION));
    m_shaders.emplace(std::piecewise_construct, std::forward_as_tuple(Visualizer::polyline),
        std::forward_as_tuple(POLYLINE_VERT_SHADER_CODE, POLYLINE_FRAG_SHADER_CODE));
}

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

DrawableMesh::DrawableMesh(std::shared_ptr<IObject> object) :
    m_vao()
{
    if (Mesh* mesh = dynamic_cast<Mesh*>(object.get())) {
        m_vao = VertexArray(*mesh);
        m_count = mesh->indexes.size();
        m_mode = GL_TRIANGLES;

        if (ColoredMesh* coloredMesh = dynamic_cast<ColoredMesh*>(object.get())) {
            VertexBuffer::Ptr vb = std::make_shared<VertexBuffer>(
                coloredMesh->colors.data(),
                static_cast<GLsizeiptr>(coloredMesh->colors.size() * sizeof(glm::vec3))
            );
            vb->addLayout(Layout{ 2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0) });
            m_vao.addVertexBuffer(vb);
        }
    }
    else if (Polyline3D* polyline = dynamic_cast<Polyline3D*>(object.get())) {
        VertexBuffer::Ptr vb = std::make_shared<VertexBuffer>(
            polyline->m_points.data(),
            static_cast<GLsizeiptr>(polyline->m_points.size() * sizeof(glm::vec3))
        );
        vb->addLayout(Layout{ 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0) });
        m_vao.addVertexBuffer(vb);
        m_count = polyline->m_points.size();
        m_mode = GL_LINE_LOOP;
    }
}

void DrawableMesh::draw() const
{
    m_vao.draw(m_mode, m_count);
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
    m_objects[visualizer].emplace_back(object);
}

void DrawingManager::cleanObjects()
{
    m_objects.clear();
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

        for (auto& obj : objects) {
            obj.draw();
        }
    }
}

bool DrawingManager::mouseMove(const ksapi::IPressedKeysPtr& pressedKeys, int32_t x, int32_t y)
{
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

#include "HighlightingManager.hpp"

#include <algorithm>
#include <memory>
#include <iostream>
#include <libloaderapi.h>
#include <wingdi.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "oglwrap/Shader.hpp"
#include "kapiwrap/DocumentFrameEvent.hpp"
#include "settings/Settings.hpp"
#include "settings/PrintSurface.hpp"
#include "shaders.hpp"
#include "settings/SettingInitializer.hpp"
#include "global.hpp"
#include "settings/DocumentsManager.hpp"
#include "settings/SettingsManager.hpp"
#include "mesh.hpp"
#include "windows.hpp"
#include "generic/math.hpp"
#include "oglwrap/VertexArray.hpp"

void* GetAnyGLFuncAddress(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void*)GetProcAddress(module, name);
    }
    return p;
}

std::unordered_map<Visualizer, ShaderProgram> HighlightingManager::m_shaders;
bool HighlightingManager::s_isGladInited = false;
short HighlightingManager::s_framesCount = 0;

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
    } else if (Polyline3D* polyline = dynamic_cast<Polyline3D*>(object.get())) {
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

HighlightingManager::HighlightingManager(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, Settings* settings) :
    DocumentFrameEvent(getDocumentFrame(kompas, document3d)), m_document3d(document3d), m_settings(settings),
    m_mode(0x00), m_mouseCoord(0, 0)
{
    // GLAD нужно инициализировать когда открыт документ
    if (!s_isGladInited) {
        if (!gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress)) {
            global::kompas->ksMessage("Ошибка инициализации GLAD");
        }
    }
    /*
      После закрытия всех документов состояние OpenGL сбрасывается.
      Поэтому, когда после этого открывается новый документ, нужно заново скомпилировать шейдеры
    */
    if (s_framesCount == 0) {
        try {
            initShaders();
        } catch (const std::runtime_error& e) {
            global::kompas->ksMessage("Ошибка компиляции шейдеров");
            std::cerr << e.what() << "\n";
        }
    }
    s_framesCount++;
}

void HighlightingManager::toggleMode(Mode mode) {
    m_mode ^= mode;
    if (mode & Mode::layersEverywhere) {
        m_mode &= ~Mode::layersAtCursor;
    } else if (mode & Mode::layersAtCursor) {
        m_mode &= ~Mode::layersEverywhere;
    }
}

void HighlightingManager::refreshWindow() const {
    m_documentFrame->RefreshWindow();
}

void HighlightingManager::addObject(std::shared_ptr<IObject> object, Visualizer visualizer)
{
    m_objects[visualizer].emplace_back(object);
}

void HighlightingManager::cleanObjects()
{
    m_objects.clear();
}

void HighlightingManager::initShaders() {
    m_shaders.emplace(std::piecewise_construct, std::forward_as_tuple(Visualizer::meshHighlight3dp),
        std::forward_as_tuple(VERTEX_SHADER_CODE, FRAGMENT_SHADER_CODE));
    m_shaders.emplace(std::piecewise_construct, std::forward_as_tuple(Visualizer::colorMesh),
        std::forward_as_tuple(VERTEX_SHADER_CODE_ORIENTATION, FRAGMENT_SHADER_CODE_ORIENTATION));
    m_shaders.emplace(std::piecewise_construct, std::forward_as_tuple(Visualizer::polyline),
        std::forward_as_tuple(POLYLINE_VERT_SHADER_CODE, POLYLINE_FRAG_SHADER_CODE));
}

kapi::IDocumentFramePtr HighlightingManager::getDocumentFrame(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d) {
    kapi::IKompasDocumentPtr document7 = kompas->TransferInterface(document3d, kapi::ksAPITypeEnum::ksAPI7Dual, 0);
    kapi::IDocumentFramesPtr documentFrames = document7->DocumentFrames;
    kapi::IDocumentFramePtr documentFrame = documentFrames->GetItem(0);
    return documentFrame;
}

bool HighlightingManager::activate() {
    return true;
}

bool HighlightingManager::closeFrame() {
    s_framesCount--;
    if (s_framesCount == 0) {
        m_shaders.clear();
    }
    global::documentsManager->remove(m_document3d);
    return true;
}

bool HighlightingManager::beginPaintGL(kapi::ksGLObject* glObject, long drawMode)
{
    /*
        Почему-то под Debug кастомная отрисовка работает только в beginPaintGL,
            а под Release только в closePaintGL.
            Так происходит при запрете стандартной отрисовки (возврат false из
            beginPaintGL).
        Оставляю включенной стандартную отрисовку. Далее в closePaintGL затираю
            результат стандартной отрисовки и вывожу кастомную. Конечно, это хуже
            по производительности, но зато единообразно.
    */
    return true;
}

bool HighlightingManager::closePaintGL(kapi::ksGLObject* glObject, long drawMode)
{
    if (m_mode != Mode::off || !m_objects.empty()) {
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

            if (visualizer == Visualizer::meshHighlight3dp) {
                if (!m_settings->isPrintSurfaceSelected())
                    continue;

                /*
                    В справке написано, что метод GetZoomScale работает только для графических документов, но для модели scale считается корректно.
                    Поэтому его и будем использовать для расчета толщины линии в шейдере (какая область вокруг точной границы слоев будет отрисовываться).
                    Чем ближе моделька, тем меньше толщина рисуемой линии.

                    (scale, lineWidth): (410.2, 0.001), (137.4, 0.003), (31.9, 0.0085), (8.9, 0.025), (4.3, 0.033)
                    аппроксимируем степенной функцией: lineWidth = 0.1192 * scale^(-0.7731)

                    Также рассчитаем радиус окружности, в пределах которой будут отрисовываться слои в режиме отрисовки у курсора

                    (scale, mouseRadius): (341.8, 450), (137.4, 220), (46.6, 110), (26.6, 90), (15.4, 60), (10.7, 70), (5.2, 40), (3.0, 20)
                    аппроксимируем степенной функцией: mouseRadius = 12.8668 * scale^(0.5943)
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

            for (auto& obj : objects) {
                obj.draw();
            }
        }
    }

    return false /* unused */;
}

bool HighlightingManager::deactivate() {
    global::settingsManager->hide();
    global::prFindOrientation->hide();
    return true;
}

bool HighlightingManager::mouseDown(short nButton, short nShiftState, long x, long y) {
    return true;
}

bool HighlightingManager::mouseMove(short nShiftState, long x, long y) {
    m_mouseCoord = glm::vec2(x, y);
    m_documentFrame->RefreshWindow();
    return true;
}

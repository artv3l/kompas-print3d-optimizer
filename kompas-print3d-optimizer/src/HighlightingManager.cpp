#include "HighlightingManager.hpp"

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
#include "utils.hpp"
#include "settings/DocumentsManager.hpp"
#include "settings/SettingsManager.hpp"
#include "mesh.hpp"

void* GetAnyGLFuncAddress(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void*)GetProcAddress(module, name);
    }
    return p;
}

std::unique_ptr<ShaderProgram> HighlightingManager::s_shaderProgram = nullptr;
bool HighlightingManager::s_isGladInited = false;
short HighlightingManager::s_framesCount = 0;

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

void HighlightingManager::initShaders() {
    s_shaderProgram = std::make_unique<ShaderProgram>(VERTEX_SHADER_CODE, FRAGMENT_SHADER_CODE);
}

kapi::IDocumentFramePtr HighlightingManager::getDocumentFrame(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d) {
    kapi::IKompasDocumentPtr document7 = kompas->TransferInterface(document3d, kapi::ksAPITypeEnum::ksAPI7Dual, 0);
    kapi::IDocumentFramesPtr documentFrames = document7->DocumentFrames;
    kapi::IDocumentFramePtr documentFrame = documentFrames->GetItem(0);
    return documentFrame;
}

void HighlightingManager::drawTriangulation(kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace) {
    kapi::ksBodyPtr body = part->GetMainBody();
    kapi::ksFaceCollectionPtr faceCollection = body->FaceCollection();
    long nFaces = faceCollection->GetCount();

    Mesh icosphere = generateIcosphere();

    GLuint vertexArrayObject; glGenVertexArrays(1, &vertexArrayObject);
    GLuint vertexBufferObject; glGenBuffers(1, &vertexBufferObject);
    GLuint elementBufferObject; glGenBuffers(1, &elementBufferObject);
    for (long iFace = 0; iFace < nFaces; iFace++) {
        kapi::ksFaceDefinitionPtr face = faceCollection->GetByIndex(iFace);

        s_shaderProgram->setUniform("u_isPrintSurface", face == printFace);

        kapi::ksTessellationPtr tesselation = face->GetTessellation();
        tesselation->refresh(); // Нужно обязательно вызывать после перестроения модели
        
        _variant_t points, indexes; tesselation->GetFacetPoints(&points, &indexes);
        _variant_t normals; tesselation->GetFacetNormals(&normals);
        if ((points.vt != (VT_ARRAY | VT_R8)) || !points.parray ||
            (indexes.vt != (VT_ARRAY | VT_I4)) || !indexes.parray ||
            (normals.vt != (VT_ARRAY | VT_R8)) || !normals.parray) {
            break;
        }
        GLsizei nPoints = points.parray->rgsabound[0].cElements - points.parray->rgsabound[0].lLbound;
        if ((points.parray->cDims != 1) || (nPoints == 0) || (nPoints % 3 != 0)) {
            break;
        }
        GLsizei nIndexes = indexes.parray->rgsabound[0].cElements - indexes.parray->rgsabound[0].lLbound;
        if ((indexes.parray->cDims != 1) || (nIndexes == 0) || (nIndexes % 3 != 0)) {
            break;
        }
        double HUGEP* pPoints = NULL; SafeArrayAccessData(points.parray, (void HUGEP * FAR*) & pPoints);
        int HUGEP* pIndexes = NULL; SafeArrayAccessData(indexes.parray, (void HUGEP * FAR*) & pIndexes);
        double HUGEP* pNormals = NULL; SafeArrayAccessData(normals.parray, (void HUGEP * FAR*) & pNormals);
        if (!pPoints || !pIndexes || !pNormals) {
            break;
        }

        std::vector<double> arrayBuffer(static_cast<size_t>(nPoints) * 2); // точки + нормали
        for (size_t i = 0; i < static_cast<size_t>(nPoints / 3); i++) {
            for (size_t j = 0; j < 3; j++) {
                arrayBuffer[(i * 6) + j] = pPoints[(i * 3) + j];
                arrayBuffer[(i * 6) + 3 + j] = pNormals[(i * 3) + j];
            }
        }

        glBindVertexArray(vertexArrayObject);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, icosphere.vertices.size() * 3 * 2 * sizeof(float), icosphere.vertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, icosphere.indices.size() * sizeof(unsigned int), icosphere.indices.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(vertexArrayObject);
        glDrawElements(GL_TRIANGLES, icosphere.indices.size(), GL_UNSIGNED_INT, 0);

        SafeArrayUnaccessData(indexes.parray);
        SafeArrayUnaccessData(points.parray);
    }

    glDeleteVertexArrays(1, &vertexArrayObject);
    glDeleteBuffers(1, &vertexBufferObject);
    glDeleteBuffers(1, &elementBufferObject);

    glUseProgram(0);
    glBindVertexArray(0);
}

bool HighlightingManager::activate() {
    return true;
}

bool HighlightingManager::closeFrame() {
    s_framesCount--;
    if (s_framesCount == 0) {
        s_shaderProgram = nullptr;
    }
    global::documentsManager->remove(m_document3d);
    return true;
}

bool HighlightingManager::beginPaintGL(kapi::ksGLObject* glObject, long drawMode) {
    if (m_mode == Mode::off) {
        return true;
    }

    glm::dvec4 dir;
    m_documentFrame->GetPickRay(0, 0, nullptr, nullptr, nullptr, &dir.x, &dir.y, &dir.z);
    kapi::ksPartPtr part = m_document3d->GetPart(kapi::Part_Type::pTop_Part);
    auto stat = calcOrientationStat(part->GetMainBody(), dir);

    glClearColor(0, stat.supportArea / stat.bodyArea, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    float matrix4[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix4);
    glm::mat4 modelview(glm::make_mat4(matrix4));
    glGetFloatv(GL_PROJECTION_MATRIX, matrix4);
    glm::mat4 projection(glm::make_mat4(matrix4));

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

    PrintSurface printSurface = m_settings->getPrintSurface();
    glm::vec3 printSurfaceNormal(printSurface.eq.a, printSurface.eq.b, printSurface.eq.c);

    s_shaderProgram->use();
    s_shaderProgram->setUniform("u_modelview", modelview);
    s_shaderProgram->setUniform("u_projection", projection);
    s_shaderProgram->setUniform("u_printSurfaceNormal", printSurfaceNormal);
    s_shaderProgram->setUniform("u_printSurfaceD", static_cast<float>(printSurface.eq.d));
    s_shaderProgram->setUniform("u_layerHeight", static_cast<float>(m_settings->getDoubleSetting(si::layerHeight.name)->getValue()));
    s_shaderProgram->setUniform("u_lineWidth", lineWidth);
    s_shaderProgram->setUniform("u_mode", m_mode);
    s_shaderProgram->setUniform("u_overhangThreshold",
                                static_cast<float>(degreeToRadian(m_settings->getDoubleSetting(si::overhangThreshold.name)->getValue())));
    s_shaderProgram->setUniform("u_mouseCoord", m_mouseCoord);
    s_shaderProgram->setUniform("u_mouseRadius", mouseRadius);

    drawTriangulation(m_document3d->GetPart(kapi::Part_Type::pTop_Part), printSurface.face);

    return false;
}

bool HighlightingManager::closePaintGL(kapi::ksGLObject* glObject, long drawMode) {
    return false /* unused */;
}

bool HighlightingManager::deactivate() {
    global::settingsManager->hide();
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

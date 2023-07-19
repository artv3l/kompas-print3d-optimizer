#include "stdafx.h"
#include "FrameEventImpl.hpp"

#include <memory>
#include <iostream>
#include <libloaderapi.h>
#include <wingdi.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "apiutil/DocumentFrameEvent.hpp"
#include "settings/Settings.hpp"
#include "settings/PrintSurface.hpp"
#include "shaders.hpp"
#include "settings/SettingInitializer.hpp"
#include "global.hpp"

void* GetAnyGLFuncAddress(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void*)GetProcAddress(module, name);
    }
    return p;
}

ShaderProgram::Ptr FrameEventImpl::s_shaderProgram = nullptr;
bool FrameEventImpl::s_isGladInited = false;
short FrameEventImpl::s_framesCount = 0;

FrameEventImpl::FrameEventImpl(KompasObjectPtr kompas, ksDocument3DPtr document3d, Settings* settings) :
    DocumentFrameEvent(getDocumentFrame(kompas, document3d)), m_document3d(document3d), m_settings(settings)
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
        } catch (const std::runtime_error&) {
            global::kompas->ksMessage("Ошибка компиляции шейдеров");
        }
    }
    s_framesCount++;
}

bool FrameEventImpl::activate() {
    return true;
}

bool FrameEventImpl::closeFrame() {
    s_framesCount--;
    if (s_framesCount == 0) {
        s_shaderProgram = nullptr;
    }
    global::documentsManager.remove(m_document3d);
    return true;
}

bool FrameEventImpl::closePaintGL(ksGLObject* glObject, long drawMode) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    float matrix4[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix4);
    glm::mat4 modelview(glm::make_mat4(matrix4));
    glGetFloatv(GL_PROJECTION_MATRIX, matrix4);
    glm::mat4 projection(glm::make_mat4(matrix4));

    PlaneEq printSurfaceEq(m_settings->getPrintSurface().eq);
    glm::vec3 printSurfaceNormal(printSurfaceEq.a, printSurfaceEq.b, printSurfaceEq.c);

    s_shaderProgram->use();
    s_shaderProgram->setUniform("u_modelview", modelview);
    s_shaderProgram->setUniform("u_projection", projection);
    s_shaderProgram->setUniform("u_printSurfaceNormal", printSurfaceNormal);
    s_shaderProgram->setUniform("u_printSurfaceD", static_cast<float>(printSurfaceEq.d));
    s_shaderProgram->setUniform("u_layerHeight", static_cast<float>(m_settings->getNumericSetting(SI_LAYER_HEIGHT.name)->getValue()));

    ksPartPtr part = m_document3d->GetPart(Part_Type::pTop_Part);
    ksBodyPtr body = part->GetMainBody();
    ksFaceCollectionPtr faceCollection = body->FaceCollection();
    long nFaces = faceCollection->GetCount();

    GLuint vertexArrayObject; glGenVertexArrays(1, &vertexArrayObject);
    GLuint vertexBufferObject; glGenBuffers(1, &vertexBufferObject);
    GLuint elementBufferObject; glGenBuffers(1, &elementBufferObject);
    for (long iFace = 0; iFace < nFaces; iFace++) {
        ksFaceDefinitionPtr face = faceCollection->GetByIndex(iFace);
        ksTessellationPtr tesselation = face->GetTessellation();
        tesselation->refresh(); // Нужно обязательно вызывать после перестроения модели

        _variant_t points, indexes; tesselation->GetFacetPoints(&points, &indexes);
        if ((points.vt != (VT_ARRAY | VT_R8)) || !points.parray || (indexes.vt != (VT_ARRAY | VT_I4)) || !indexes.parray) {
            break;
        }
        GLuint nPoints = points.parray->rgsabound[0].cElements - points.parray->rgsabound[0].lLbound;
        if ((points.parray->cDims != 1) || (nPoints == 0) || (nPoints % 3 != 0)) {
            break;
        }
        GLuint nIndexes = indexes.parray->rgsabound[0].cElements - indexes.parray->rgsabound[0].lLbound;
        if ((indexes.parray->cDims != 1) || (nIndexes == 0) || (nIndexes % 3 != 0)) {
            break;
        }
        double HUGEP* pPoints = NULL; SafeArrayAccessData(points.parray, (void HUGEP * FAR*) & pPoints);
        int HUGEP* pIndexes = NULL; SafeArrayAccessData(indexes.parray, (void HUGEP * FAR*) & pIndexes);
        if (!pPoints || !pIndexes) {
            break;
        }

        glBindVertexArray(vertexArrayObject);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
            glBufferData(GL_ARRAY_BUFFER, nPoints * sizeof(double), pPoints, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndexes * sizeof(int), pIndexes, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 3 * sizeof(double), (void*)0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glBindVertexArray(vertexArrayObject);
        glDrawElements(GL_TRIANGLES, nIndexes, GL_UNSIGNED_INT, 0);

        SafeArrayUnaccessData(indexes.parray);
        SafeArrayUnaccessData(points.parray);
    }

    glDeleteVertexArrays(1, &vertexArrayObject);
    glDeleteBuffers(1, &vertexBufferObject);
    glDeleteBuffers(1, &elementBufferObject);

    glUseProgram(0);
    glBindVertexArray(0);

    return true;
}

bool FrameEventImpl::deactivate() {
    global::settingsManager.hide();
    return true;
}

bool FrameEventImpl::mouseDown(short nButton, short nShiftState, long x, long y) {
    return true;
}

void FrameEventImpl::initShaders() {
    Shader vertexShader(VERTEX_SHADER_CODE, GL_VERTEX_SHADER);
    Shader fragmentShader(FRAGMENT_SHADER_CODE, GL_FRAGMENT_SHADER);
    s_shaderProgram = ShaderProgram::link({&vertexShader, &fragmentShader});
}

IDocumentFramePtr FrameEventImpl::getDocumentFrame(KompasObjectPtr kompas, ksDocument3DPtr document3d) {
    IKompasDocumentPtr document7 = kompas->TransferInterface(document3d, ksAPITypeEnum::ksAPI7Dual, 0);
    IDocumentFramesPtr documentFrames = document7->DocumentFrames;
    IDocumentFramePtr documentFrame = documentFrames->GetItem(0);
    return documentFrame;
}

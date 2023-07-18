#include "stdafx.h"
#include "FrameEventImpl.hpp"

#include <iostream>
#include <memory>

#include <libloaderapi.h>
#include <wingdi.h>

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "apiutil/DocumentFrameEvent.hpp"
#include "shaders.hpp"

void* GetAnyGLFuncAddress(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void*)GetProcAddress(module, name);
    }
    return p;
}

ShaderProgram::Ptr FrameEventImpl::s_shaderProgram = nullptr;

FrameEventImpl::FrameEventImpl(IDocumentFramePtr documentFrame, ksPartPtr part) :
    DocumentFrameEvent(documentFrame), m_part(part)
{
    if (!s_shaderProgram) {
        if (!gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress)) {
            std::cerr << "Failed to initialize GLAD" << "\n";
            // todo: fatal error
        }
        std::cout << "GLAD init: ok\n";
        try {
            initShaders();
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << "\n";
            // todo: fatal error
        }
        std::cout << "Shader compile: ok\n";
    } else {
        std::cout << "GLAD already inited\n";
    }
}

bool FrameEventImpl::activate() {
    return true;
}

bool FrameEventImpl::closePaintGL(ksGLObject* glObject, long drawMode) {
    std::cout << "closePaintGL" << "\n";

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    float matrix4[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix4);
    glm::mat4 modelview(glm::make_mat4(matrix4));
    glGetFloatv(GL_PROJECTION_MATRIX, matrix4);
    glm::mat4 projection(glm::make_mat4(matrix4));

    s_shaderProgram->use();
    s_shaderProgram->setUniform("u_modelview", modelview);
    s_shaderProgram->setUniform("u_projection", projection);

    ksBodyPtr body = m_part->GetMainBody();
    ksFaceCollectionPtr faceCollection = body->FaceCollection();
    long nFaces = faceCollection->GetCount();

    GLuint vertexArrayObject; glGenVertexArrays(1, &vertexArrayObject);
    GLuint vertexBufferObject; glGenBuffers(1, &vertexBufferObject);
    GLuint elementBufferObject; glGenBuffers(1, &elementBufferObject);
    for (long iFace = 0; iFace < nFaces; iFace++) {
        ksFaceDefinitionPtr face = faceCollection->GetByIndex(iFace);
        ksTessellationPtr tesselation = face->GetTessellation();

        _variant_t points, indexes; tesselation->GetFacetPoints(&points, &indexes);
        int HUGEP* pIndexes = NULL; SafeArrayAccessData(indexes.parray, (void HUGEP * FAR*) & pIndexes);
        double HUGEP* pPoints = NULL; SafeArrayAccessData(points.parray, (void HUGEP * FAR*) & pPoints);

        GLuint nPoints = points.parray->rgsabound[0].cElements - points.parray->rgsabound[0].lLbound;
        GLuint nIndexes = indexes.parray->rgsabound[0].cElements - indexes.parray->rgsabound[0].lLbound;

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

    glUseProgram(0);
    glBindVertexArray(0);

    return true;
}

void FrameEventImpl::initShaders() {
    Shader vertexShader(VERTEX_SHADER_CODE, GL_VERTEX_SHADER);
    Shader fragmentShader(FRAGMENT_SHADER_CODE, GL_FRAGMENT_SHADER);
    s_shaderProgram = ShaderProgram::link({&vertexShader, &fragmentShader});
}

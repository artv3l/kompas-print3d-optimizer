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

FrameEventImpl::FrameEventImpl(IDocumentFramePtr documentFrame) :
    DocumentFrameEvent(documentFrame)
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

    float matrix4[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix4);
    //glm::mat4 modelview(glm::make_mat4(matrix4));
    glGetFloatv(GL_PROJECTION_MATRIX, matrix4);
    //glm::mat4 projection(glm::make_mat4(matrix4));

    return false;
}

void FrameEventImpl::initShaders() {
    Shader vertexShader(VERTEX_SHADER_CODE, GL_VERTEX_SHADER);
    Shader fragmentShader(FRAGMENT_SHADER_CODE, GL_FRAGMENT_SHADER);
    s_shaderProgram = ShaderProgram::link({&vertexShader, &fragmentShader});
}

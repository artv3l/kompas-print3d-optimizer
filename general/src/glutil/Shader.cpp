#include "glutil/Shader.hpp"

#include <fstream>
#include <stdexcept>
#include <array>
#include <vector>
#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


const GLsizei INFO_LOG_SIZE = 512;

Shader::Shader(const char* code, GLenum type) {
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &code, nullptr);
    glCompileShader(id);
    checkStatus(id, GL_COMPILE_STATUS);
    m_id = id;
}

Shader::~Shader() {
    glDeleteShader(m_id);
}

GLuint Shader::getId() const {
    return m_id;
}

std::string Shader::readCodeFromFile(const char* filepath) {
    const size_t BLOCK_SIZE = 4096;

    std::ifstream fin(filepath);
    if (!fin) {
        throw std::runtime_error("File does not exist");
    }
    std::string shaderCode;
    std::string buf(BLOCK_SIZE, '\0');
    while (fin.read(&buf[0], BLOCK_SIZE)) {
        shaderCode.append(buf, 0, fin.gcount());
    }
    shaderCode.append(buf, 0, fin.gcount());
    return shaderCode;
}

void Shader::checkStatus(GLuint id, GLenum parameter) {
    GLint isSuccess = 0; glGetShaderiv(id, parameter, &isSuccess);
    if (isSuccess == 0) {
        std::array<char, INFO_LOG_SIZE> infoLog;
        glGetShaderInfoLog(id, INFO_LOG_SIZE, nullptr, infoLog.data());
        throw std::runtime_error("Shader compilation failed: " + std::string(infoLog.data()));
    }
}

ShaderProgram::ShaderProgram(GLuint id):
    m_id(id)
{
    assert(id != 0);
}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(m_id);
}

ShaderProgram::Ptr ShaderProgram::link(std::vector<const Shader*> shaders) {
    GLuint id = glCreateProgram();
    for (const Shader* shader : shaders) {
        glAttachShader(id, shader->getId());
    }
    glLinkProgram(id);
    checkStatus(id, GL_LINK_STATUS);
    return std::make_unique<ShaderProgram>(id);
}

void ShaderProgram::use() const {
    glUseProgram(m_id);
}

void ShaderProgram::setUniform(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void ShaderProgram::setUniform(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void ShaderProgram::setUniform(const std::string& name, glm::mat4 mat4) const {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat4));
}

void ShaderProgram::setUniform(const std::string& name, glm::vec3 vec3) const {
    glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(vec3));
}

void ShaderProgram::checkStatus(GLuint id, GLenum parameter) {
    GLint isSuccess = 0; glGetProgramiv(id, parameter, &isSuccess);
    if (isSuccess == 0) {
        std::array<char, INFO_LOG_SIZE> infoLog;
        glGetProgramInfoLog(id, INFO_LOG_SIZE, nullptr, infoLog.data());
        throw std::runtime_error("Shader program linking failed: " + std::string(infoLog.data()));
    }
}

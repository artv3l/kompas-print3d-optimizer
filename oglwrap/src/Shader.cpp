#include "Shader.hpp"

#include <fstream>
#include <stdexcept>
#include <array>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

const GLsizei INFO_LOG_SIZE = 512;

Shader::Shader(const std::string& src, GLenum type):
    m_id(glCreateShader(type))
{
    const char* tmp = src.c_str();
    glShaderSource(m_id, 1, &tmp, nullptr);
    glCompileShader(m_id);
    checkStatus(m_id, GL_COMPILE_STATUS);
}

Shader::Shader(Shader&& obj) noexcept:
    m_id(obj.m_id)
{}

Shader::~Shader() {
    glDeleteShader(m_id);
}

std::string Shader::load(const std::string& filepath) {
    static const size_t BLOCK_SIZE = 4096;

    std::ifstream fin(filepath);
    if (!fin) {
        throw std::runtime_error("File does not exist");
    }

    std::string src;
    std::string buf(BLOCK_SIZE, '\0');
    while (fin.read(buf.data(), BLOCK_SIZE)) {
        src.append(buf, 0, fin.gcount());
    }
    src.append(buf, 0, fin.gcount());

    return src;
}

GLuint Shader::getId() const {
    return m_id;
}

void Shader::checkStatus(GLuint id, GLenum parameter) {
    GLint isSuccess = 0; glGetShaderiv(id, parameter, &isSuccess);
    if (isSuccess == 0) {
        std::array<char, INFO_LOG_SIZE> infoLog;
        glGetShaderInfoLog(id, INFO_LOG_SIZE, nullptr, infoLog.data());
        throw std::runtime_error("Shader compilation failed: " + std::string(infoLog.data()));
    }
}

ShaderProgram::ShaderProgram(std::string vertexSrc, std::string fragmentSrc):
    m_id(glCreateProgram())
{
    Shader vert(vertexSrc, GL_VERTEX_SHADER);
    Shader frag(fragmentSrc, GL_FRAGMENT_SHADER);
    glAttachShader(m_id, vert.getId());
    glAttachShader(m_id, frag.getId());
    glLinkProgram(m_id);
    checkStatus(m_id, GL_LINK_STATUS);
}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(m_id);
}

ActionLock ShaderProgram::use() const {
    GLint prev = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prev);
    glUseProgram(m_id);
    return ActionLock([prev]() { glUseProgram(prev); });
}

void ShaderProgram::setUniform(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void ShaderProgram::setUniform(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void ShaderProgram::setUniform(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
}

void ShaderProgram::setUniform(const std::string& name, glm::mat4 mat4) const {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat4));
}

void ShaderProgram::setUniform(const std::string& name, glm::vec2 vec2) const {
    glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(vec2));
}

void ShaderProgram::setUniform(const std::string& name, glm::vec3 vec3) const {
    glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(vec3));
}

void ShaderProgram::setUniform(const std::string& name, glm::vec4 vec4) const {
    glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(vec4));
}

void ShaderProgram::checkStatus(GLuint id, GLenum parameter) {
    GLint isSuccess = 0; glGetProgramiv(id, parameter, &isSuccess);
    if (isSuccess == 0) {
        std::array<char, INFO_LOG_SIZE> infoLog;
        glGetProgramInfoLog(id, INFO_LOG_SIZE, nullptr, infoLog.data());
        throw std::runtime_error("Shader program linking failed: " + std::string(infoLog.data()));
    }
}

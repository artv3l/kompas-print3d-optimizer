#pragma once

#include <string>
#include <variant>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "generic/ActionLock.hpp"

using UniformName = std::string;
using UniformData = std::variant<int, glm::vec3>;
using Uniforms = std::unordered_map<UniformName, UniformData>;

class Shader {
public:
    Shader(const std::string& src, GLenum type);
    Shader(const Shader& obj) = delete;
    Shader(Shader&& obj) noexcept;
    ~Shader();

    Shader& operator=(const Shader& obj) = delete;
    Shader& operator=(Shader&& obj) noexcept = delete;

    static std::string load(const std::string& filepath);

    GLuint getId() const;

private:
    GLuint m_id;

    static void checkStatus(GLuint id, GLenum parameter);
};

class ShaderProgram {
public:
    ShaderProgram(std::string vertexSrc, std::string fragmentSrc);
    ShaderProgram(const ShaderProgram& obj) = delete;
    ShaderProgram(ShaderProgram&& obj) noexcept = default;
    ~ShaderProgram();

    ShaderProgram& operator=(const ShaderProgram& obj) = delete;
    ShaderProgram& operator=(ShaderProgram&& obj) noexcept = default;

    ActionLock use() const;
    void setUniform(const std::string& name, float value) const;
    void setUniform(const std::string& name, int value) const;
    void setUniform(const std::string& name, bool value) const;
    void setUniform(const std::string& name, glm::mat4 mat4) const;
    void setUniform(const std::string& name, glm::vec2 vec2) const;
    void setUniform(const std::string& name, glm::vec3 vec3) const;
    void setUniform(const std::string& name, glm::vec4 vec4) const;
    void setUniforms(const Uniforms& uniforms) const;

private:
    GLuint m_id;

    static void checkStatus(GLuint id, GLenum parameter);
};

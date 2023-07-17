#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <vector>
#include <memory>

#include "glad/glad.h"
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const char* code, GLenum type);
    Shader(const Shader& obj) = delete;
    Shader(Shader&& obj) noexcept = delete;
    ~Shader();

    Shader& operator=(const Shader& obj) = delete;
    Shader& operator=(Shader&& obj) noexcept = delete;

    GLuint getId() const;

private:
    GLuint m_id;

    static std::string readCodeFromFile(const char* filepath);
    static void checkStatus(GLuint id, GLenum parameter);
};

class ShaderProgram {
public:
    using Ptr = std::unique_ptr<ShaderProgram>;

    ShaderProgram(GLuint id);
    ShaderProgram(const ShaderProgram& obj) = delete;
    ShaderProgram(ShaderProgram&& obj) noexcept = delete;
    ~ShaderProgram();

    ShaderProgram& operator=(const ShaderProgram& obj) = delete;
    ShaderProgram& operator=(ShaderProgram&& obj) noexcept = delete;

    static Ptr link(std::vector<const Shader*> shaders);

    void use() const;
    void setUniform(const std::string& name, float value) const;
    void setUniform(const std::string& name, int value) const;
    void setUniform(const std::string& name, glm::mat4 mat4) const;

private:
    GLuint m_id;

    static void checkStatus(GLuint id, GLenum parameter);
};

#endif /* SHADER_HPP */

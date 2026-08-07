#include "oglwrap/VertexBuffer.hpp"

#include <unordered_map>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "oglwrap/Mesh.hpp"

Buffer::~Buffer() {
    glDeleteBuffers(1, &m_id);
}

void Buffer::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::VertexBuffer(const void* data, GLsizeiptr size):
    m_layouts()
{
    glGenBuffers(1, &m_id);
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    unbind();
}

void VertexBuffer::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBuffer::addLayout(const Layout& layout) {
    m_layouts.push_back(layout);
}

const std::vector<Layout>& VertexBuffer::getLayouts() const {
    return m_layouts;
}

ElementBuffer::ElementBuffer(const GLuint* indices, GLsizeiptr count) {
    glGenBuffers(1, &m_id);
    m_count = count;
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), indices, GL_STATIC_DRAW);
    unbind();
}

void ElementBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

GLsizeiptr ElementBuffer::getCount() const {
    return m_count;
}

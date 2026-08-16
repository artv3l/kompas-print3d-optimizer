#include "oglwrap/VertexArray.hpp"

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_id);
}

ActionLock VertexArray::bind() const {
    GLint prev = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prev);
    glBindVertexArray(m_id);
    return ActionLock([prev]() { glBindVertexArray(prev); });
}

void VertexArray::setElementBuffer(const ElementBuffer::Ptr& elementBuffer) {
    auto lock = bind();
    elementBuffer->bind();
    m_elementBuffer = elementBuffer;
    elementBuffer->unbind();
}

void VertexArray::addVertexBuffer(const VertexBuffer::Ptr& vertexBuffer) {
    auto lock = bind();
    vertexBuffer->bind();
    for (const Layout& layout : vertexBuffer->getLayouts()) {
        glVertexAttribPointer(layout.index, layout.size, layout.type, layout.normalized, layout.stride, layout.pointer);
        glEnableVertexAttribArray(layout.index); 
    }
    m_vertexBuffers.push_back(vertexBuffer);
    vertexBuffer->unbind();
}

void VertexArray::draw(GLenum mode) const {
    auto lock = bind();
    glDrawElements(mode, static_cast<GLsizei>(m_elementBuffer->getCount()), GL_UNSIGNED_INT, 0);
}

void VertexArray::draw(GLenum mode, size_t count) const
{
    auto lock = bind();
    if (m_elementBuffer)
        glDrawElements(mode, static_cast<GLsizei>(count), GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(mode, 0, static_cast<GLsizei>(count));
}

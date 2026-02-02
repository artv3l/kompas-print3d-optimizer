#include "VertexArray.hpp"

void VertexArray::unbind() {
    glBindVertexArray(0);
}

VertexArray::VertexArray(const Mesh& mesh) {
    glGenVertexArrays(1, &m_id);

    {
        VertexBuffer::Ptr vb = std::make_shared<VertexBuffer>(
            mesh.positions.data(),
            static_cast<GLsizeiptr>(mesh.positions.size() * sizeof(glm::vec3))
        );
        vb->addLayout(Layout{0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0)});
        addVertexBuffer(vb);
    }
    {
        VertexBuffer::Ptr vb = std::make_shared<VertexBuffer>(
            mesh.normals.data(),
            static_cast<GLsizeiptr>(mesh.normals.size() * sizeof(glm::vec3))
        );
        vb->addLayout(Layout{1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0) });
        addVertexBuffer(vb);
    }

    ElementBuffer::Ptr eb = std::make_shared<ElementBuffer>(mesh.indexes.data(), static_cast<GLsizeiptr>(mesh.indexes.size()));
    setElementBuffer(eb);
}

void VertexArray::bind() const {
    glBindVertexArray(m_id);
}

void VertexArray::setElementBuffer(const ElementBuffer::Ptr& elementBuffer) {
    bind();
    elementBuffer->bind();
    m_elementBuffer = elementBuffer;
    unbind();
    elementBuffer->unbind();
}

void VertexArray::addVertexBuffer(const VertexBuffer::Ptr& vertexBuffer) {
    bind();
    vertexBuffer->bind();
    for (const Layout& layout : vertexBuffer->getLayouts()) {
        glVertexAttribPointer(layout.index, layout.size, layout.type, layout.normalized, layout.stride, layout.pointer);
        glEnableVertexAttribArray(layout.index); 
    }
    m_vertexBuffers.push_back(vertexBuffer);
    unbind();
    vertexBuffer->unbind();
}

void VertexArray::draw(GLenum mode) const {
    bind();
    glDrawElements(mode, static_cast<GLsizei>(m_elementBuffer->getCount()), GL_UNSIGNED_INT, 0);
    unbind();
}

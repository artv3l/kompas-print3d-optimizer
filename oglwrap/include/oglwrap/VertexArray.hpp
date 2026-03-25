#pragma once

#include <vector>
#include <memory>

#include <glad/glad.h>

#include "ActionLock.hpp"
#include "VertexBuffer.hpp"
#include "Mesh.hpp"

class VertexArray {
public:
    using Ptr = std::shared_ptr<VertexArray>;

public:
    VertexArray();
    VertexArray(const Mesh& mesh);

    [[nodiscard]] ActionLock bind() const;
    void setElementBuffer(const ElementBuffer::Ptr& elementBuffer);
    void addVertexBuffer(const VertexBuffer::Ptr& vertexBuffer);

    void draw(GLenum mode) const;
    void draw(GLenum mode, size_t count) const;

private:
    GLuint m_id;
    std::vector<VertexBuffer::Ptr> m_vertexBuffers;
    ElementBuffer::Ptr m_elementBuffer;
};

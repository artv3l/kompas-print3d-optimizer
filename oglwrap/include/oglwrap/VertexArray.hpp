#pragma once

#include <vector>
#include <memory>

#include <glad/glad.h>

#include "generic/ActionLock.hpp"
#include "oglwrap/buffer/VertexBuffer.hpp"
#include "oglwrap/Mesh.hpp"

class VertexArray {
public:
    using Ptr = std::shared_ptr<VertexArray>;

public:
    VertexArray();

    [[nodiscard]] ActionLock bind() const;
    void setElementBuffer(const ElementBuffer::Ptr& elementBuffer);
    void addVertexBuffer(const VertexBuffer::Ptr& vertexBuffer);

    void draw(GLenum mode) const;
    void draw(GLenum mode, size_t count) const;

private:
    GLuint m_id = 0;
    std::vector<VertexBuffer::Ptr> m_vertexBuffers;
    ElementBuffer::Ptr m_elementBuffer;
};

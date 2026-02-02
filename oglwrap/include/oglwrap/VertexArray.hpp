#pragma once

#include <vector>
#include <memory>

#include <glad/glad.h>

#include "VertexBuffer.hpp"
#include "Mesh.hpp"

class VertexArray {
public:
    using Ptr = std::shared_ptr<VertexArray>;

    static void unbind();

public:
    VertexArray(const Mesh& mesh);

    void bind() const;
    void setElementBuffer(const ElementBuffer::Ptr& elementBuffer);
    void addVertexBuffer(const VertexBuffer::Ptr& vertexBuffer);

    void draw(GLenum mode) const;

private:
    GLuint m_id;
    std::vector<VertexBuffer::Ptr> m_vertexBuffers;
    ElementBuffer::Ptr m_elementBuffer;
};

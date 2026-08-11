#pragma once

#include "oglwrap/buffer/Buffer.hpp"

Buffer::Buffer()
{
    glGenBuffers(1, &m_id);
}

Buffer::~Buffer() {
    glDeleteBuffers(1, &m_id);
}

void Buffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

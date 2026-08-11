#pragma once

#include <glad/glad.h>

class Buffer {
public:
    Buffer();
    virtual ~Buffer();

    virtual void unbind() const;
    virtual void bind() const = 0;
protected:
    GLuint m_id = 0;
};

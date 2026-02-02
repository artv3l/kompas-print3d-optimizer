#pragma once

#include <vector>
#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>

struct Layout {
    GLuint index;
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    const void* pointer;
};

class Buffer {
public:
    virtual ~Buffer();

    static void unbind();
    virtual void bind() const = 0;
protected:
    GLuint m_id;
};

class VertexBuffer : public Buffer {
public:
    using Ptr = std::shared_ptr<VertexBuffer>;

    VertexBuffer(const void* data, GLsizeiptr size);
    VertexBuffer(const VertexBuffer& obj) = delete;
    VertexBuffer(VertexBuffer&& obj) noexcept = delete;

    virtual ~VertexBuffer() override = default;

    VertexBuffer& operator=(const VertexBuffer& obj) = delete;
    VertexBuffer& operator=(VertexBuffer&& obj) noexcept = delete;

    void bind() const override;
    void addLayout(const Layout& layout);
    const std::vector<Layout>& getLayouts() const;

private:
    std::vector<Layout> m_layouts;
};

class ElementBuffer : public Buffer {
public:
    using Ptr = std::shared_ptr<ElementBuffer>;

    ElementBuffer(const GLuint* indices, GLsizeiptr count);
    ElementBuffer(const ElementBuffer& obj) = delete;
    ElementBuffer(ElementBuffer&& obj) noexcept = delete;

    virtual ~ElementBuffer() override = default;

    ElementBuffer& operator=(const ElementBuffer& obj) = delete;
    ElementBuffer& operator=(ElementBuffer&& obj) noexcept = delete;

    void bind() const override;

    GLsizeiptr getCount() const;

private:
    GLsizeiptr m_count;
};

#include "oglwrap/buffer/ShaderStorageBuffer.hpp"

#include <glad/glad.h>

ShaderStorageBuffer::ShaderStorageBuffer(GLuint bindingPoint)
	: Buffer()
	, m_bindingPoint(bindingPoint)
{
}

void ShaderStorageBuffer::unbind() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ShaderStorageBuffer::bind() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindingPoint, m_id);
}

void ShaderStorageBuffer::uploadData(const void* data, size_t size) const
{
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

#pragma once

#include "oglwrap/buffer/Buffer.hpp"

class ShaderStorageBuffer final : public Buffer
{
public:
	ShaderStorageBuffer(GLuint bindingPoint);
	ShaderStorageBuffer(const ShaderStorageBuffer& other) = delete;
	ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept = delete;

	ShaderStorageBuffer& operator=(const ShaderStorageBuffer& other) = delete;
	ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other) noexcept = delete;

	void unbind() const override;
	void bind() const override;

	void uploadData(const void * data, size_t size) const;

private:
	GLuint m_bindingPoint;
};

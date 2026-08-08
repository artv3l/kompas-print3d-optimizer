#pragma once

#include <memory>

#include "oglwrap/Mesh.hpp"
#include "oglwrap/VertexArray.hpp"
#include "oglwrap/Shader.hpp"

class DrawableMesh
{
public:
    DrawableMesh(std::shared_ptr<IObject> object, Uniforms uniforms);
    virtual ~DrawableMesh() = default;
    void draw(const ShaderProgram& shaderProgram) const;
protected:
    VertexArray m_vao;
    size_t m_count; // РљРѕР»-РІРѕ РёРЅРґРµРєСЃРѕРІ РґР»СЏ РѕС‚СЂРёСЃРѕРІРєРё
    int m_mode;
    Uniforms m_uniforms;
};

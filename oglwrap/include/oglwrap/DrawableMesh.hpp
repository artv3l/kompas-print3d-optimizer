#pragma once

#include <memory>

#include "oglwrap/Mesh.hpp"
#include "oglwrap/VertexArray.hpp"
#include "oglwrap/Shader.hpp"
#include "oglwrap/buffer/ShaderStorageBuffer.hpp"

/*
  РћР±СЉРµРєС‚, РєРѕС‚РѕСЂС‹Р№ РјРѕР¶РЅРѕ СЂРёСЃРѕРІР°С‚СЊ. Р”Р°РЅРЅС‹Рµ РЅР° СЃС‚РѕСЂРѕРЅРµ GPU
*/
struct IDrawableObject
{
    virtual ~IDrawableObject() = default;
    
    virtual void draw(const ShaderProgram& shaderProgram) const = 0;
};

class DrawableObject : public IDrawableObject
{
protected:
    VertexArray m_vao;
    size_t m_count = 0; // РљРѕР»-РІРѕ РёРЅРґРµРєСЃРѕРІ РґР»СЏ РѕС‚СЂРёСЃРѕРІРєРё
    Uniforms m_uniforms;
};

class DrawableColoredMesh : public DrawableObject
{
public:
    DrawableColoredMesh(const ColoredMesh& coloredMesh);
    void draw(const ShaderProgram& shaderProgram) const override;

private:
    ShaderStorageBuffer m_ssboColors;
};

class DrawablePolyline3D : public DrawableObject
{
public:
    DrawablePolyline3D(const Polyline3D& polyline3d);
    void draw(const ShaderProgram& shaderProgram) const override;
};

std::unique_ptr<IDrawableObject> createDrawableMesh(std::shared_ptr<IObject> object);

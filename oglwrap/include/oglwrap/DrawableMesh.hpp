#pragma once

#include <memory>

#include "oglwrap/Mesh.hpp"
#include "oglwrap/VertexArray.hpp"

class DrawableMesh
{
public:
    DrawableMesh(std::shared_ptr<IObject> object);
    virtual ~DrawableMesh() = default;
    void draw() const;
protected:
    VertexArray m_vao;
    size_t m_count; // РљРѕР»-РІРѕ РёРЅРґРµРєСЃРѕРІ РґР»СЏ РѕС‚СЂРёСЃРѕРІРєРё
    int m_mode;
};

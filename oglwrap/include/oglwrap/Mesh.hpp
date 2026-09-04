#pragma once

#include <vector>
#include <span>

#include <glm/glm.hpp>

#include "generic/geometry3d.hpp"
#include "generic/color.hpp"

/*
  РћР±СЉРµРєС‚, РєРѕС‚РѕСЂС‹Р№ РјРѕР¶РЅРѕ СЂРёСЃРѕРІР°С‚СЊ (РµРіРѕ РґР°РЅРЅС‹Рµ РЅР° СЃС‚РѕСЂРѕРЅРµ CPU).
  Р’СЃРµ РґР°РЅРЅС‹Рµ СѓР¶Рµ РІРѕ float Рё РІ СѓРґРѕР±РЅРѕРј РґР»СЏ Р·Р°РіСЂСѓР·РєРё РЅР° РІРёРґРµРѕРєР°СЂС‚Сѓ С„РѕСЂРјР°С‚Рµ
*/
struct IObject
{
    virtual ~IObject() = default;
};

class Mesh : public IObject
{
public:
    using Index = unsigned int;

    Mesh(const geom3d::Mesh& mesh);
    ~Mesh() override = default;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<Index> indexes;
};

class ColoredMesh : public Mesh
{
public:
    enum class ColorType
    {
        byTriangle,
        byVertex,
    };

public:
    ColoredMesh(const geom3d::Mesh& mesh, const color::RGB& color, ColorType colorType);
    ~ColoredMesh() override = default;

    std::vector<glm::vec4> colors;
    ColorType m_colorType;
};

class Polyline3D : public IObject
{
public:
    Polyline3D(std::span<const geom3d::Vec3> points, const color::RGB& color);
    ~Polyline3D() override = default;

    std::vector<glm::vec3> m_points;
    glm::vec3 m_color;
};

void transform(std::span<glm::vec3> data, glm::mat4 matrix);

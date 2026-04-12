#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "generic/geometry2d.hpp"

using Index = unsigned int;

// РћР±СЉРµРєС‚, РєРѕС‚РѕСЂС‹Р№ РјРѕР¶РЅРѕ СЂРёСЃРѕРІР°С‚СЊ. Р’СЃРµ РґР°РЅРЅС‹Рµ СѓР¶Рµ РІРѕ float Рё РІ СѓРґРѕР±РЅРѕРј РґР»СЏ Р·Р°РіСЂСѓР·РєРё РЅР° РІРёРґРµРѕРєР°СЂС‚Сѓ С„РѕСЂРјР°С‚Рµ
class IObject
{
public:
    virtual ~IObject() = default;
};

class Mesh : public IObject
{
public:
    ~Mesh() override = default;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<Index> indexes;
};

class ColoredMesh : public Mesh
{
public:
    ~ColoredMesh() override = default;

    std::vector<glm::vec3> colors;
};

class Polyline3D : public IObject
{
public:
    ~Polyline3D() override = default;

    std::vector<glm::vec3> m_points;
};

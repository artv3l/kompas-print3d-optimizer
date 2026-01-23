#include "mesh.hpp"

#include <cassert>

#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>

Mesh generateIcosphere()
{
    auto icosphere = Magnum::Primitives::icosphereSolid(3);
    
    auto positions = icosphere.positions3DAsArray();
    auto normals = icosphere.normalsAsArray();
    if (positions.size() != normals.size())
    {
        assert(false);
        return {};
    }

    Mesh result;
    auto position = positions.cbegin();
    auto normal = normals.cbegin();
    while (position != positions.cend() && normal != normals.cend())
    {
        Vertex vertex;
        vertex.position = glm::vec3(position->x(), position->y(), position->z());
        vertex.normal = glm::vec3(normal->x(), normal->y(), normal->z());
        result.vertices.emplace_back(std::move(vertex));

        ++position;
        ++normal;
    }

    for (auto&& i : icosphere.indicesAsArray())
        result.indices.emplace_back(i);

    return result;
}

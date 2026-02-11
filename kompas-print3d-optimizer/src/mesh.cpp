#include "mesh.hpp"

#include <cassert>
#include <algorithm>
#include <vector>
#include <iterator>

#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>

Mesh generateIcosphere()
{
    auto icosphere = Magnum::Primitives::icosphereSolid(2);
    
    auto positions = icosphere.positions3DAsArray();
    auto normals = icosphere.normalsAsArray();
    if (positions.size() != normals.size())
    {
        assert(false);
        return {};
    }

    auto toGlmVec = [](const Magnum::Vector3 & vec3) { return glm::vec3(vec3.x(), vec3.y(), vec3.z()); };

    Mesh result;

    result.positions.reserve(positions.size());
    std::transform(positions.begin(), positions.end(), std::back_inserter(result.positions), toGlmVec);

    result.normals.reserve(normals.size());
    std::transform(normals.begin(), normals.end(), std::back_inserter(result.normals), toGlmVec);

    auto indexes = icosphere.indicesAsArray();
    result.indexes.reserve(indexes.size());
    std::copy(indexes.begin(), indexes.end(), std::back_inserter(result.indexes));

    return result;
}

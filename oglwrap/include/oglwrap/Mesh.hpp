#pragma once

#include <vector>

#include <glm/glm.hpp>

using Index = unsigned int;

struct Mesh final {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<Index> indexes;
    // colors
};

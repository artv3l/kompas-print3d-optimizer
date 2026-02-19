#pragma once

#include <vector>

#include <glm/glm.hpp>

using Index = unsigned int;

struct Mesh
{
    virtual ~Mesh() = default;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<Index> indexes;
};

struct ColoredMesh : public Mesh
{
    std::vector<glm::vec3> colors;
};

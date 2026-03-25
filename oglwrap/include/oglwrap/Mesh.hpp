#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "generic/geometry2d.hpp"

using Index = unsigned int;

class Mesh : public geometry::IObject
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

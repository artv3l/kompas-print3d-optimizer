#ifndef MESH_HPP
#define MESH_HPP

#include <vector>

#include <glm/glm.hpp>

struct Vertex final {
    glm::vec3 position;
    glm::vec3 normal;
};

struct Mesh final {
    std::vector<Vertex> vertices;
    std::vector<size_t> indices;
};

Mesh generateIcosphere();

#endif /* MESH_HPP */

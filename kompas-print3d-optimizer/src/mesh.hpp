#pragma once

#include <vector>
#include <span>

#include <glm/glm.hpp>

#include "oglwrap/Mesh.hpp"
#include "generic/geometry2d.hpp"

Mesh generateIcosphere(uint8_t subdivisionsCount);
geometry::Polygon convexHull(std::span<glm::vec2> points);

Mesh copyToMesh(kapi::ksTessellationPtr tessellation);
Mesh copyToMesh(kapi::ksBodyPtr body);

#pragma once

#include <vector>
#include <span>

#include <glm/glm.hpp>

#include "oglwrap/Mesh.hpp"
#include "generic/geometry2d.hpp"

Mesh generateIcosphere();
geometry::Polygon convexHull(std::span<glm::vec2> points);

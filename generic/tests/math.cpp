#include <gtest/gtest.h>

#include "generic/math.hpp"

TEST(generic_math, isOnPrintPlane_1)
{
    const math::Plane plane(glm::vec3(35, -25, 32), glm::vec3(-46, 6, 31), glm::vec3(-11, 65, -25));
    const math::Triangle triangle(
        glm::vec3(47.879179, -6.819262, 12.791762), // Расстояние до plane = 1.0
        glm::vec3(35.249851, -2.0, 14.0), // Лежит на plane
        glm::vec3(39.711088, -10.0, 20.0) // Расстояние до plane = 0.8
    );

    EXPECT_TRUE(isOnPrintPlane(triangle, plane, 1.1));
    EXPECT_FALSE(isOnPrintPlane(triangle, plane, 0.9));
}

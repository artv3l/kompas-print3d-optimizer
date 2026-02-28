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
    // Угол между плоскостью plane и triangle: A = 2.975471 рад. = 170.481905 град., pi - A = 0.166122 рад. = 9.518095 град.

    EXPECT_TRUE(isOnPrintPlane(triangle, plane, math::toRadians(15), 1.2));
    EXPECT_TRUE(isOnPrintPlane(triangle, plane, math::toRadians(11), 1.1));

    EXPECT_FALSE(isOnPrintPlane(triangle, plane, math::toRadians(9), 1.2));
    EXPECT_FALSE(isOnPrintPlane(triangle, plane, math::toRadians(15), 0.8));
}

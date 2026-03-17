#include <gtest/gtest.h>

#include "generic/math.hpp"

TEST(generic_math, Triangle_area)
{
    const math::Triangle triangle(
        glm::vec3(293, 96, 130),
        glm::vec3(235, 61, 275),
        glm::vec3(186, 139, 208)
    );

    EXPECT_TRUE(math::equal(triangle.area(), 7747.554888, 0.01));
}

TEST(generic_math, project_1)
{
    const math::Triangle triangle(
        glm::vec3(293, 96, 130),
        glm::vec3(235, 61, 275),
        glm::vec3(186, 139, 208)
    );
    const math::Plane plane(triangle);

    const glm::dvec3 p1(169.962101, 119.792824, 171.173376);
    const glm::dvec3 p2(222, 197, 212);

    EXPECT_TRUE(math::equal(math::distance(p1, plane), 37.731074));
    EXPECT_TRUE(math::equal(math::distance(p2, plane), 63.579672));
}

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

TEST(generic_math, volumeUnderOverhang_1)
{
    const math::Triangle overhang(
        glm::vec3(293, 96, 130),
        glm::vec3(235, 61, 275),
        glm::vec3(186, 139, 208)
    );
    const math::Plane printPlane(math::Triangle(
        glm::vec3(44, -32, 41),
        glm::vec3(-70, -2, 75),
        glm::vec3(12, 81, -4)
    ));

    EXPECT_TRUE(math::equal(volumeUnderOverhang(printPlane, overhang), 1715913.984499, 0.5));
}

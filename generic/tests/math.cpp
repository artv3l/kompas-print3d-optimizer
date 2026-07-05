#include <gtest/gtest.h>

#include "generic/math.hpp"

TEST(generic_math, Triangle_area)
{
    const geom3d::Triangle triangle(
        geom3d::Vec3(293, 96, 130),
        geom3d::Vec3(235, 61, 275),
        geom3d::Vec3(186, 139, 208)
    );

    EXPECT_TRUE(math::equal(geom3d::triangleArea(triangle), 7747.554888, 0.01));
}

TEST(generic_math, project_1)
{
    const geom3d::Plane plane = geom3d::Plane::Through(
        geom3d::Vec3(293, 96, 130),
        geom3d::Vec3(235, 61, 275),
        geom3d::Vec3(186, 139, 208)
    );

    const geom3d::Vec3 p1(169.962101, 119.792824, 171.173376);
    const geom3d::Vec3 p2(222, 197, 212);

    EXPECT_TRUE(math::equal(plane.absDistance(p1), 37.731074));
    EXPECT_TRUE(math::equal(plane.absDistance(p2), 63.579672));
}

TEST(generic_math, worldToLocal_1)
{
    const geom3d::Vec3 origin(39.0, -33.0, 47.0);
    const geom3d::Placement local(
        origin,
        geom3d::Vec3(0.926979, 0.124504, 0.353849),
        geom3d::Vec3(0.309146, 0.280714, -0.908641),
        geom3d::Vec3(-0.212460, 0.951682, 0.221726)
    );
    const Eigen::Affine3d mat = local.matrixToWorld().inverse();

    const geom3d::Vec3 point(70.000389, -37.081360, 26.571628);
    const geom3d::Vec3 pointExpected(21.0, 27.0, -15.0);
    const geom3d::Vec3 pointActual = mat * point;

    EXPECT_TRUE(
        math::equal(pointActual.x(), pointExpected.x(), 0.0001) &&
        math::equal(pointActual.y(), pointExpected.y(), 0.0001) &&
        math::equal(pointActual.z(), pointExpected.z(), 0.0001)
    );
}

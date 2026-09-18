#include <gtest/gtest.h>

#include "generic/math.hpp"
#include "generic/geometry3d.hpp"

TEST(generic_geometry3d, triangleArea_1)
{
    const geom3d::Triangle triangle(
        geom3d::Vec3(293, 96, 130),
        geom3d::Vec3(235, 61, 275),
        geom3d::Vec3(186, 139, 208)
    );

    EXPECT_TRUE(math::equal(geom3d::triangleArea(triangle), 7747.554888, 0.01));
}

TEST(generic_geometry3d, project_1)
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

TEST(generic_geometry3d, worldToLocal_1)
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

TEST(generic_geometry3d, angleBetween_1)
{
    const geom3d::Vec3 vec1 = geom3d::Vec3(-17, 25, 36) - geom3d::Vec3(0, 0, 0);
    const geom3d::Vec3 vec2 = geom3d::Vec3(-38, 59, 24) - geom3d::Vec3(-9, 29, 17);
    const geom3d::Vec3 vec3 = geom3d::Vec3(-45, 30, 30) - geom3d::Vec3(-9, 29, 17);
    const geom3d::Vec3 vec4 = geom3d::Vec3(15, 6, -33) - geom3d::Vec3(-9, 29, 17);

    EXPECT_TRUE(math::equal(geom3d::angleBetween(vec1, vec2), 0.720228));
    EXPECT_TRUE(math::equal(geom3d::angleBetween(vec2, vec3), 0.768187));
    EXPECT_TRUE(math::equal(geom3d::angleBetween(vec3, vec4), 2.30309));
}

TEST(generic_geometry3d_Placement, createByAxisZ)
{
    const geom3d::Vec3 origin(28, 120, 62);
    const geom3d::Vec3 axisZ(0.863221, 0.451531, 0.225765);
    const geom3d::Placement placement2 = geom3d::Placement::createByAxisZ(origin, axisZ);
    EXPECT_TRUE(
        placement2.getOrigin().isApprox(origin, math::c_epsilon) &&
        placement2.getAxisX().isApprox(geom3d::Vec3(0.504826, -0.772088, -0.386043), math::c_epsilon) &&
        placement2.getAxisY().isApprox(geom3d::Vec3(0, 0.447212, -0.894427), math::c_epsilon) &&
        placement2.getAxisZ().isApprox(axisZ, math::c_epsilon)
    );
}

TEST(generic_geometry3d, calcGabarit_1)
{
    const geom3d::Mesh mesh {
        .positions = {
            geom3d::Vec3(191, 0, 26),
            geom3d::Vec3(-8, 20, 41),
            geom3d::Vec3(-36, 73, -7),
            geom3d::Vec3(2, 211, -81),
            geom3d::Vec3(-21, 42, -23),
            geom3d::Vec3(41, -3, -14),
        },
        .normals = { // Р”Р»СЏ calcGabarit РЅРѕСЂРјР°Р»Рё РЅРµ РІР°Р¶РЅС‹
            geom3d::Vec3(0, 0, 0),
            geom3d::Vec3(0, 0, 0),
            geom3d::Vec3(0, 0, 0),
            geom3d::Vec3(0, 0, 0),
            geom3d::Vec3(0, 0, 0),
            geom3d::Vec3(0, 0, 0),
        },
        .indexes = {
            0, 1, 5,
            1, 4, 5,
            1, 2, 4,
            2, 3, 4,
        }
    };

    {
        const geom3d::Gabarit gab1_expected(
            geom3d::Vec3(-36, -3, -81),
            geom3d::Vec3(191, 211, 41)
        );
        const geom3d::Gabarit gab1_actual = geom3d::calcGabarit(mesh, geom3d::Placement::createDefault());
        EXPECT_TRUE(
            gab1_expected.min().isApprox(gab1_actual.min()) &&
            gab1_expected.max().isApprox(gab1_actual.max())
        );
    }
    {
        const geom3d::Placement placement2 = geom3d::Placement::createByAxisZ(
            geom3d::Vec3(28, 120, 62),
            geom3d::Vec3(0.863221, 0.451531, 0.225765)
        );
        const geom3d::Gabarit gab2_expected(
            geom3d::Vec3(-28.181340, -25.938301, -96.707244),
            geom3d::Vec3(188.834918, 168.599510, 78.393740)
        );
        const geom3d::Gabarit gab2_actual = geom3d::calcGabarit(mesh, placement2);
        EXPECT_TRUE(
            gab2_expected.min().isApprox(gab2_actual.min(), math::c_epsilon) &&
            gab2_expected.max().isApprox(gab2_actual.max(), math::c_epsilon)
        );
    }
}

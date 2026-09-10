#include <gtest/gtest.h>

#include "core/orientation/orientation.hpp"

TEST(core_orientation, toRelative_1)
{
    std::vector<double> empty;
    EXPECT_EQ(toRelative(empty), empty);
    std::vector<double> one = { 2 };
    EXPECT_EQ(toRelative(one), empty);
}

TEST(core_orientation, toRelative_2)
{
    auto equal = [](double a, double b) { return math::equal(a, b); };

    std::vector<double> values_1 = {2, 3, 3.5, 4, 7};
    std::vector<double> expected_1 = {0, 0.2, 0.3, 0.4, 1};
    EXPECT_TRUE(std::ranges::equal((toRelative(values_1)), expected_1, equal));

    std::vector<double> values_2 = { 1, 6, -3.5, 3, -4, 0.5, 5 };
    std::vector<double> expected_2 = { 0.5, 1, 0.05, 0.7, 0, 0.45, 0.9 };
    EXPECT_TRUE(std::ranges::equal((toRelative(values_2)), expected_2, equal));
}

TEST(core_orientation, calcOrientationStatByMesh_badMesh)
{
    geom3d::Mesh emptyMesh;
    EXPECT_EQ(calcOrientationStatByMesh(emptyMesh, 45.0, 1), nullptr);

    geom3d::Mesh badMesh1;
    badMesh1.positions = { geom3d::Vec3(0, 0, 0), geom3d::Vec3(1, 1, 1), geom3d::Vec3(2, 2, 2) };
    badMesh1.normals = { geom3d::Vec3(0, 0, 0), geom3d::Vec3(1, 1, 1), geom3d::Vec3(2, 2, 2), geom3d::Vec3(3, 3, 3) };
    badMesh1.indexes = { 1, 2, 3 };
    EXPECT_EQ(calcOrientationStatByMesh(badMesh1, 45.0, 1), nullptr);

    geom3d::Mesh badMesh2;
    badMesh2.positions = { geom3d::Vec3(0, 0, 0), geom3d::Vec3(1, 1, 1), geom3d::Vec3(2, 2, 2) };
    badMesh2.normals = { geom3d::Vec3(0, 0, 0), geom3d::Vec3(1, 1, 1), geom3d::Vec3(2, 2, 2) };
    badMesh2.indexes = { 1, 2, 3, 4 };
    EXPECT_EQ(calcOrientationStatByMesh(badMesh2, 45.0, 1), nullptr);
}

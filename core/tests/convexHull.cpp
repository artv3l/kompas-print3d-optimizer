#include <vector>

#include <gtest/gtest.h>

#include "core/convexHull.hpp"
#include "generic/geometry3d.hpp"

TEST(core_convexHull, corner_case_1)
{
    const std::vector<Eigen::Vector2d> points1{ Eigen::Vector2d(1, 2) };
    EXPECT_TRUE(convexHull(points1) == points1);
    const std::vector<Eigen::Vector2d> points2{ Eigen::Vector2d(1, 2), Eigen::Vector2d(1, 2), Eigen::Vector2d(1, 2) };
    EXPECT_TRUE(convexHull(points2) == points1);
    const std::vector<Eigen::Vector2d> empty{ };
    EXPECT_TRUE(convexHull(empty) == empty);
}

TEST(core_convexHull, corner_case_2)
{
    const std::vector<Eigen::Vector2d> points1{ Eigen::Vector2d(1, 1), Eigen::Vector2d(4, 4) };
    EXPECT_TRUE(convexHull(points1) == points1);
    const std::vector<Eigen::Vector2d> points2{ Eigen::Vector2d(1, 1), Eigen::Vector2d(4, 4), Eigen::Vector2d(1, 1), Eigen::Vector2d(4, 4) };
    EXPECT_TRUE(convexHull(points2) == points1);
    const std::vector<Eigen::Vector2d> points3{ Eigen::Vector2d(1, 1), Eigen::Vector2d(2, 2), Eigen::Vector2d(3, 3), Eigen::Vector2d(4, 4) };
    EXPECT_TRUE(convexHull(points3) == points1);
}

TEST(core_convexHull, corner_case_3)
{
    const std::vector<Eigen::Vector2d> points1{ Eigen::Vector2d(1, 2), Eigen::Vector2d(3, 2), Eigen::Vector2d(4, 3) };
    EXPECT_TRUE(convexHull(points1) == points1);
    const std::vector<Eigen::Vector2d> points2{ Eigen::Vector2d(1, 2), Eigen::Vector2d(3, 2), Eigen::Vector2d(4, 3), Eigen::Vector2d(3, 2), Eigen::Vector2d(1, 2) };
    EXPECT_TRUE(convexHull(points2) == points1);
    const std::vector<Eigen::Vector2d> points3{ Eigen::Vector2d(1, 2), Eigen::Vector2d(3, 2), Eigen::Vector2d(4, 3), Eigen::Vector2d(2, 2) };
    EXPECT_TRUE(convexHull(points3) == points1);
}

TEST(core_convexHull, simple)
{
    const std::vector<Eigen::Vector2d> points1{
        Eigen::Vector2d(1, 2), Eigen::Vector2d(2, 6), Eigen::Vector2d(8, -4), Eigen::Vector2d(-1, -1),
        Eigen::Vector2d(-3, 4), Eigen::Vector2d(0, -3), Eigen::Vector2d(4, 0), Eigen::Vector2d(3, 3),
        Eigen::Vector2d(-4, 0),
    };
    const std::vector<Eigen::Vector2d> expected{
        Eigen::Vector2d(-4, 0), Eigen::Vector2d(0, -3), Eigen::Vector2d(8, -4), Eigen::Vector2d(2, 6),
        Eigen::Vector2d(-3, 4),
    };
    EXPECT_TRUE(convexHull(points1) == expected);

    const std::vector<Eigen::Vector2d> points1_shuffle{
        Eigen::Vector2d(-3, 4), Eigen::Vector2d(8, -4), Eigen::Vector2d(1, 2), Eigen::Vector2d(3, 3),
        Eigen::Vector2d(-4, 0), Eigen::Vector2d(2, 6), Eigen::Vector2d(4, 0), Eigen::Vector2d(-1, -1),
        Eigen::Vector2d(0, -3),
    };
    EXPECT_TRUE(convexHull(points1_shuffle) == expected);
}

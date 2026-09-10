#include <gtest/gtest.h>

#include "generic/math.hpp"

TEST(generic_math, convertRanges_1)
{
    EXPECT_TRUE(math::equal(math::convertRanges(3, 0, 9, 1, 3), 2));
    EXPECT_TRUE(math::equal(math::convertRanges(0, -2, 4, 7, 1), 7.5));
    EXPECT_TRUE(math::equal(math::convertRanges(6.1, 6, 1, 600, 100), 610));
    EXPECT_TRUE(math::equal(math::convertRanges(1.5, 4, 5, -20, 7), -23.5));
}

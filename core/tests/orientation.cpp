#include <gtest/gtest.h>

#include "core/orientation/orientation.hpp"

TEST(core_orientation, isOnPrintPlane_1)
{
    const geom3d::Plane plane = geom3d::Plane::Through(
        geom3d::Vec3(35, -25, 32),
        geom3d::Vec3(-46, 6, 31),
        geom3d::Vec3(-11, 65, -25)
    );
    const geom3d::Triangle triangle = {
        geom3d::Vec3(47.879179, -6.819262, 12.791762), // Р Р°СЃСЃС‚РѕСЏРЅРёРµ РґРѕ plane = 1.0
        geom3d::Vec3(35.249851, -2.0, 14.0), // Р›РµР¶РёС‚ РЅР° plane
        geom3d::Vec3(39.711088, -10.0, 20.0) // Р Р°СЃСЃС‚РѕСЏРЅРёРµ РґРѕ plane = 0.8
    };

    EXPECT_TRUE(isOnPrintPlane(triangle, plane, 1.1));
    EXPECT_FALSE(isOnPrintPlane(triangle, plane, 0.9));
}

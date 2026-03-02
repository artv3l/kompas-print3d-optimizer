#pragma once

#include <cstdint>

namespace color
{
enum class StandardColor
{
    red,
    green,
};

struct RGB final
{
    double red;   // 0..1
    double green; // 0..1
    double blue;  // 0..1
};

struct HSV final
{
    double hue;        // 0..360
    double saturation; // 0..1
    double value;      // 0..1
};

RGB toRGB(HSV in);

template <typename T, StandardColor color>
T getStandardColor();
}

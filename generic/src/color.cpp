#include "color.hpp"

namespace color
{
RGB toRGB(HSV in)
{
    // https://stackoverflow.com/a/6930407

    double      hh, p, q, t, ff;
    long        i;
    RGB         out;

    if (in.saturation <= 0.0) {       // < is bogus, just shuts up warnings
        out.red = in.value;
        out.green = in.value;
        out.blue = in.value;
        return out;
    }
    hh = in.hue;
    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.value * (1.0 - in.saturation);
    q = in.value * (1.0 - (in.saturation * ff));
    t = in.value * (1.0 - (in.saturation * (1.0 - ff)));

    switch (i) {
    case 0:
        out.red = in.value;
        out.green = t;
        out.blue = p;
        break;
    case 1:
        out.red = q;
        out.green = in.value;
        out.blue = p;
        break;
    case 2:
        out.red = p;
        out.green = in.value;
        out.blue = t;
        break;

    case 3:
        out.red = p;
        out.green = q;
        out.blue = in.value;
        break;
    case 4:
        out.red = t;
        out.green = p;
        out.blue = in.value;
        break;
    case 5:
    default:
        out.red = in.value;
        out.green = p;
        out.blue = q;
        break;
    }
    return out;
}

template<> HSV getStandardColor<HSV, StandardColor::red>() {
    return HSV{ 0.0, 1.0, 1.0 }; }
template<> RGB getStandardColor<RGB, StandardColor::red>() {
    return RGB{ 1.0, 0.0, 0.0 }; }
template<> HSV getStandardColor<HSV, StandardColor::green>() {
    return HSV{ 120.0, 1.0, 1.0 }; }
template<> RGB getStandardColor<RGB, StandardColor::green>() {
    return RGB{ 0.0, 1.0, 0.0 }; }
template<> RGB getStandardColor<RGB, StandardColor::blue>() {
    return RGB{ 0.0, 0.0, 1.0 };
}
}

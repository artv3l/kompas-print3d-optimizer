#ifndef PRINT_SURFACE_HPP
#define PRINT_SURFACE_HPP

#include <utility>

struct PlaneEq {
    double a, b, c, d;

    PlaneEq(ksFaceDefinitionPtr face);

    bool operator==(const PlaneEq& other) const;
    bool operator!=(const PlaneEq& other) const;

    void invert();
};

struct PrintSurface {
    ksFaceDefinitionPtr face;
    PlaneEq eq;
};

std::pair<int, int> countPointsOnEachSide(ksPartPtr part, const PlaneEq& planeEq);

PrintSurface getSelectedPrintSurface(ksDocument3DPtr document3d);

#endif /* PRINT_SURFACE_HPP */

#ifndef SELECT_PLANE_HPP
#define SELECT_PLANE_HPP

#include "stdafx.h"

const double PLANE_BORDER_EPS = 0.001;

class PlaneEq {
public:
    double a, b, c, d;
    bool isVertical(ksEdgeDefinitionPtr edge, double cos_angle);
    bool equals(PlaneEq other);
    PlaneEq(ksFaceDefinitionPtr face);
    PlaneEq();
};

struct PrintPlane {
    ksFaceDefinitionPtr face;
    PlaneEq eq;
};

void checkPlane(KompasObjectPtr kompas, double a, double b, double c, double d, int* s1, int* s2);

PrintPlane getSelectedPlane(KompasObjectPtr kompas);

#endif /* SELECT_PLANE_HPP */

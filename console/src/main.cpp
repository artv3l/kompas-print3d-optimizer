#include "stdafx.h"

#include <iostream>
#include <cstdlib>
#include <float.h>

#define _USE_MATH_DEFINES \ #include <cmath>

#include "connection.hpp"
#include "PrintSurface.hpp"
#include "optimization/rounding.hpp"
#include "optimization/elephantFoot.hpp"
#include "optimization/circleHorizontalHoles.hpp"
#include "optimization/bridgeHole.hpp"
#include "optimization/roundingEdgesOnPrintFace.hpp"

int main() {
    CoInitialize(nullptr);
    KompasObjectPtr kompas = kompasInit();
    if (!kompas) {
        return 0;
    }
    ksDocument3DPtr document3d = kompas->ActiveDocument3D();
    ksPartPtr part = document3d->GetPart(pTop_Part);

    PrintSurface printSurface = getSelectedPrintSurface(document3d);

    return 0;
}

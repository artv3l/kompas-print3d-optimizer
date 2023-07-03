#include "stdafx.h"

#include <iostream>
#include <cstdlib>
#include <float.h>
#include <list>

#define _USE_MATH_DEFINES \ #include <cmath>

#include "connection.hpp"
#include "settings/PrintSurface.hpp"
#include "optimization/rounding.hpp"
#include "optimization/elephantFoot.hpp"
#include "optimization/circleHorizontalHoles.hpp"
#include "optimization/bridgeHole.hpp"
#include "optimization/roundingEdgesOnPrintFace.hpp"
#include "settings/DocumentData.hpp"

int main() {
    CoInitialize(nullptr);
    KompasObjectPtr kompas = nullptr;
    try {
        kompas = kompasInit();
    } catch (const std::runtime_error& e) {
        std::cout << e.what() << "\n";
        return 2;
    }

    ksDocument3DPtr document3d = kompas->ActiveDocument3D();
    ksChooseMngPtr chooseMng = document3d->GetChooseMng();
    ksPartPtr part = document3d->GetPart(pTop_Part);
    ksBodyPtr body = part->GetMainBody();

    PrintSurface printSurface(getSelectedPrintSurface(document3d));
    DocumentData::Settings settings(document3d);
    settings.setPrintSurface(printSurface);


    CoUninitialize();
    return 0;
}

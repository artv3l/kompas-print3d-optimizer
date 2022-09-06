#include "stdafx.h"
#include "utils.hpp"

#include <cmath>

bool doubleEqual(double a, double b, double epsilon) {
    return (abs(a - b) < epsilon);
}

Sketch createSketch(KompasObjectPtr kompas, ksPartPtr part, IDispatchPtr plane) {
    ksEntityPtr sketchEntity(part->NewEntity(o3d_sketch));
    ksSketchDefinitionPtr sketchDef(sketchEntity->GetDefinition());
    sketchDef->SetPlane(plane);
    sketchEntity->Create();
    ksDocument2DPtr sketchEdit(sketchDef->BeginEdit());
    IKompasDocument2DPtr sketchEdit_api7(kompas->TransferInterface(sketchEdit, ksAPI7Dual, 0));

    return Sketch{ sketchEntity, sketchDef, sketchEdit, sketchEdit_api7 };
}

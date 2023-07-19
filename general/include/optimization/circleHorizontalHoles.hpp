#ifndef CIRCLE_HORIZONTAL_HOLES_HPP
#define CIRCLE_HORIZONTAL_HOLES_HPP

#include <list>

#include "apiutil/Macro.hpp"
#include "settings/Setting.hpp"

class Settings;

ksEntityPtr createConeFaceAxis(ksPartPtr part, ksFaceDefinitionPtr coneFace, bool hidden = true);
ksEntityPtr createPlanePerpendicular(ksPartPtr part, ksEntityPtr axis, ksEntityPtr point, bool hidden = true);
ksEntityPtr createPlaneLineToPlane(ksPartPtr part, ksEntityPtr line, ksEntityPtr plane, bool isParallel, bool hidden = true);
IPoint3DPtr createPointCenter(IPart7Ptr part7, IFacePtr face7, bool hidden = true);
ksEntityPtr createCutExtrusion(ksPartPtr part, Sketch sketch);

ICirclePtr createBaseCircle(Sketch sketch, ksFaceDefinitionPtr target, _bstr_t& out_radiusVariable);
void drawTriangle(Sketch sketch, ICirclePtr baseCircle, ILinePtr verticalLine, _bstr_t radiusVariable, NumericSetting::Ptr overhangThreshold, double rotationOffset);
void drawSketch(Sketch sketch, ksFaceDefinitionPtr target, ksEntityPtr verticalPlane, NumericSetting::Ptr overhangThreshold);
Macro buildHoleTriangle(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace, ksFaceDefinitionPtr target, NumericSetting::Ptr overhangThreshold);

std::list<ksFaceDefinitionPtr> getCircleHorizontalHoleTargets(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace);
ksEntityPtr optimizeCircleHorizontalHoles(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, Settings& settings);

#endif /* CIRCLE_HORIZONTAL_HOLES_HPP */

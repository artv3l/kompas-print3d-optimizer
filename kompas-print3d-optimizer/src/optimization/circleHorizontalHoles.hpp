#ifndef CIRCLE_HORIZONTAL_HOLES_HPP
#define CIRCLE_HORIZONTAL_HOLES_HPP

#include <list>

#include "kapiwrap/Sketch.hpp"
#include "kapiwrap/Macro.hpp"

#include "settings/Setting.hpp"

class Settings;

kapi::ksEntityPtr createConeFaceAxis(kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr coneFace, bool hidden = true);
kapi::ksEntityPtr createPlanePerpendicular(kapi::ksPartPtr part, kapi::ksEntityPtr axis, kapi::ksEntityPtr point, bool hidden = true);
kapi::ksEntityPtr createPlaneLineToPlane(kapi::ksPartPtr part, kapi::ksEntityPtr line, kapi::ksEntityPtr plane, bool isParallel, bool hidden = true);
kapi::IPoint3DPtr createPointCenter(kapi::IPart7Ptr part7, kapi::IFacePtr face7, bool hidden = true);
kapi::ksEntityPtr createCutExtrusion(kapi::ksPartPtr part, Sketch sketch);

kapi::ICirclePtr createBaseCircle(Sketch sketch, kapi::ksFaceDefinitionPtr target, _bstr_t& out_radiusVariable);
void drawTriangle(Sketch sketch, kapi::ICirclePtr baseCircle, kapi::ILinePtr verticalLine, _bstr_t radiusVariable, DoubleSetting::Ptr overhangThreshold, double rotationOffset);
void drawSketch(Sketch sketch, kapi::ksFaceDefinitionPtr target, kapi::ksEntityPtr verticalPlane, DoubleSetting::Ptr overhangThreshold);
Macro buildHoleTriangle(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace, kapi::ksFaceDefinitionPtr target, DoubleSetting::Ptr overhangThreshold);

std::list<kapi::ksFaceDefinitionPtr> getCircleHorizontalHoleTargets(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace);
kapi::ksEntityPtr optimizeCircleHorizontalHoles(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, kapi::ksPartPtr part, Settings& settings);

#endif /* CIRCLE_HORIZONTAL_HOLES_HPP */

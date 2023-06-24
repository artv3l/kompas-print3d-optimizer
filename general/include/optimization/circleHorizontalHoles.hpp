#ifndef CIRCLE_HORIZONTAL_HOLES_HPP
#define CIRCLE_HORIZONTAL_HOLES_HPP

#include <list>

#include "PrintSurface.hpp"
#include "SettingsManager.hpp"

ksEntityPtr createConeFaceAxis(ksPartPtr part, ksFaceDefinitionPtr coneFace, bool hidden = true);
ksEntityPtr createPlanePerpendicular(ksPartPtr part, ksEntityPtr axis, ksEntityPtr point, bool hidden = true);
IPoint3DPtr createPointCenter(IPart7Ptr part7, IFacePtr face7, bool hidden = true);
void buildHoleTriangle(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr target);

std::list<ksFaceDefinitionPtr> getCircleHorizontalHoleTargets(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, const Settings& settings);

#endif /* CIRCLE_HORIZONTAL_HOLES_HPP */

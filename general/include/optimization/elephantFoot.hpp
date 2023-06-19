#ifndef OPTIMIZE_ELEPHANT_FOOT_HPP
#define OPTIMIZE_ELEPHANT_FOOT_HPP

#include <list>

#include "PrintSurface.hpp"

std::list<ksLoopPtr> getElephantFootTargets(ksPartPtr part, PrintSurface printSurface);
void createElephantFootChamfers(ksPartPtr part, std::list<ksLoopPtr> elephantFootTargets, double width);

void optimizeElephantFoot(ksPartPtr part, PrintSurface printSurface, double width);

#endif /* OPTIMIZE_ELEPHANT_FOOT_HPP */

#ifndef ELEPHANT_FOOT_HPP
#define ELEPHANT_FOOT_HPP

#include <list>

#include "PrintSurface.hpp"
#include "apiutil/SettingsManager.hpp"

std::list<ksLoopPtr> getElephantFootTargets(ksPartPtr part, PrintSurface printSurface);
void createElephantFootChamfers(ksPartPtr part, std::list<ksLoopPtr> elephantFootTargets, double width);

void optimizeElephantFoot(ksPartPtr part, PrintSurface printSurface, const Settings& settings);

#endif /* ELEPHANT_FOOT_HPP */

#ifndef ELEPHANT_FOOT_HPP
#define ELEPHANT_FOOT_HPP

#include <list>

#include "PrintSurface.hpp"
#include "SettingsManager.hpp"

std::list<ksLoopPtr> getElephantFootTargets(ksPartPtr part, PrintSurface printSurface);
void createElephantFootChamfers(ksPartPtr part, std::list<ksLoopPtr> elephantFootTargets, double width);

size_t optimizeElephantFoot(ksPartPtr part, const Settings& settings);

#endif /* ELEPHANT_FOOT_HPP */

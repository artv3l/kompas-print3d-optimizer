#ifndef ELEPHANT_FOOT_HPP
#define ELEPHANT_FOOT_HPP

#include <list>

#include "settings/PrintSurface.hpp"

class Settings;

std::list<ksLoopPtr> getElephantFootTargets(ksPartPtr part, PrintSurface printSurface);
ksEntityPtr createElephantFootChamfers(ksPartPtr part, std::list<ksLoopPtr> elephantFootTargets, Settings& settings);

ksEntityPtr optimizeElephantFoot(ksPartPtr part, Settings& settings);

#endif /* ELEPHANT_FOOT_HPP */

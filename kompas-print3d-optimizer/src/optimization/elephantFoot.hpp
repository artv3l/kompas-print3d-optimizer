#ifndef ELEPHANT_FOOT_HPP
#define ELEPHANT_FOOT_HPP

#include <list>

#include "settings/PrintSurface.hpp"

class Settings;

std::list<kapi::ksLoopPtr> getElephantFootTargets(kapi::ksPartPtr part, PrintSurface printSurface);
kapi::ksEntityPtr createElephantFootChamfers(kapi::ksPartPtr part, std::list<kapi::ksLoopPtr> elephantFootTargets, Settings& settings);

kapi::ksEntityPtr optimizeElephantFoot(kapi::ksPartPtr part, Settings& settings);

#endif /* ELEPHANT_FOOT_HPP */

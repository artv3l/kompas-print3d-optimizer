#ifndef ELEPHANT_FOOT_HPP
#define ELEPHANT_FOOT_HPP

#include <list>

#include "settings/PrintSurface.hpp"
#include "settings/DocumentData.hpp"

std::list<ksLoopPtr> getElephantFootTargets(ksPartPtr part, PrintSurface printSurface);
ksEntityPtr createElephantFootChamfers(ksPartPtr part, std::list<ksLoopPtr> elephantFootTargets, DocumentData::Settings& settings);

ksEntityPtr optimizeElephantFoot(ksPartPtr part, DocumentData::Settings& settings);

#endif /* ELEPHANT_FOOT_HPP */

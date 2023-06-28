#ifndef ELEPHANT_FOOT_HPP
#define ELEPHANT_FOOT_HPP

#include <list>
#include <utility>

#include "PrintSurface.hpp"
#include "SettingsManager.hpp"
#include "apiutil/Macro.hpp"

std::list<ksLoopPtr> getElephantFootTargets(ksPartPtr part, PrintSurface printSurface);
Macro createElephantFootChamfers(ksPartPtr part, std::list<ksLoopPtr> elephantFootTargets, double width);

std::pair<size_t, Optional<Macro>> optimizeElephantFoot(ksPartPtr part, const Settings& settings);

#endif /* ELEPHANT_FOOT_HPP */

#ifndef ROUNDING_HPP
#define ROUNDING_HPP

#include <list>

#include "SettingsManager.hpp"

bool isEdgeForRounding(ksEdgeDefinitionPtr edge, ksPartPtr part, ksFaceDefinitionPtr printFace, double deflectionAngle);

std::list<ksEdgeDefinitionPtr> getRoundingTargets(ksPartPtr part, ksFaceDefinitionPtr printFace, double deflectionAngle);
void roundEdges(ksPartPtr part, std::list<ksEdgeDefinitionPtr> roundingTargets, double radius);

void optimizeRounding(ksPartPtr part, ksFaceDefinitionPtr printFace, const Settings& settings);

#endif /* ROUNDING_HPP */

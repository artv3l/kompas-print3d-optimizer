#ifndef ROUNDING_HPP
#define ROUNDING_HPP

#include <list>

class Settings;

bool isEdgeForRounding(kapi::ksEdgeDefinitionPtr edge, kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace, double deflectionAngle);

std::list<kapi::ksEdgeDefinitionPtr> getRoundingTargets(kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace, double deflectionAngle);
kapi::ksEntityPtr roundEdges(kapi::ksPartPtr part, std::list<kapi::ksEdgeDefinitionPtr> roundingTargets, double radius);

kapi::ksEntityPtr optimizeRounding(kapi::ksPartPtr part, Settings& settings);

#endif /* ROUNDING_HPP */

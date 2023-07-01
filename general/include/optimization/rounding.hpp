#ifndef ROUNDING_HPP
#define ROUNDING_HPP

#include <list>
#include <utility>

#include "settings/DocumentData.hpp"

bool isEdgeForRounding(ksEdgeDefinitionPtr edge, ksPartPtr part, ksFaceDefinitionPtr printFace, double deflectionAngle);

std::list<ksEdgeDefinitionPtr> getRoundingTargets(ksPartPtr part, ksFaceDefinitionPtr printFace, double deflectionAngle);
ksEntityPtr roundEdges(ksPartPtr part, std::list<ksEdgeDefinitionPtr> roundingTargets, double radius);

std::pair<size_t, Optional<ksEntityPtr>> optimizeRounding(ksPartPtr part, DocumentData::Settings& settings);

#endif /* ROUNDING_HPP */

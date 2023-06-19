#ifndef OPTIMIZE_ROUNDING_HPP
#define OPTIMIZE_ROUNDING_HPP

#include <list>

bool isEdgeForRounding(ksEdgeDefinitionPtr edge, ksPartPtr part, ksFaceDefinitionPtr printFace, double deflectionAngle);

std::list<ksEdgeDefinitionPtr> getRoundingTargets(ksPartPtr part, ksFaceDefinitionPtr printFace, double deflectionAngle);
void roundEdges(ksPartPtr part, std::list<ksEdgeDefinitionPtr> roundingTargets, double radius);

void optimizeRounding(ksPartPtr part, ksFaceDefinitionPtr printFace, double radius, double deflectionAngle);

#endif /* OPTIMIZE_ROUNDING_HPP */

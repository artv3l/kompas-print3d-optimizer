#ifndef ROUNDING_EDGES_ON_PRINT_FACE_HPP
#define ROUNDING_EDGES_ON_PRINT_FACE_HPP

#include <list>

#include "kapiwrap/Sketch.hpp"

#include "settings/PrintSurface.hpp"
#include "settings/Setting.hpp"

class Settings;

enum class ReworkType {
    ALL,
    ONLY_WITH_REWORK,
    ONLY_WITHOUT_REWORK,
};

struct RoundingEdgeOnPrintFaceTarget {
    std::list<kapi::ksEdgeDefinitionPtr> trajectory;
    kapi::ksFaceDefinitionPtr roundingFace;
    bool needRework;
};

double getCylinderOrTorusRadius(kapi::ksFaceDefinitionPtr face);
bool faceNeedRework(kapi::ksFaceDefinitionPtr roundingFace);
bool targetNeedRework(RoundingEdgeOnPrintFaceTarget target);

std::list<RoundingEdgeOnPrintFaceTarget> getRoundingEdgesOnPrintFaceTargets(kapi::ksPartPtr part, PrintSurface printSurface, ReworkType reworkType);
void drawSketch(Sketch sketch, RoundingEdgeOnPrintFaceTarget target, DoubleSetting::Ptr overhangThreshold);
kapi::ksEntityPtr optimizeRoundingEdgesOnPrintFace(kapi::KompasObjectPtr kompas, kapi::ksPartPtr part, Settings& settings, ReworkType reworkType, size_t& reworkCount);

#endif /* ROUNDING_EDGES_ON_PRINT_FACE_HPP */

#ifndef ROUNDING_EDGES_ON_PRINT_FACE_HPP
#define ROUNDING_EDGES_ON_PRINT_FACE_HPP

#include <list>

#include "PrintSurface.hpp"
#include "utils.hpp"
#include "apiutil/SettingsManager.hpp"

enum class ReworkType {
    ALL,
    ONLY_WITH_REWORK,
    ONLY_WITHOUT_REWORK,
};

struct RoundingEdgeOnPrintFaceTarget {
    std::list<ksEdgeDefinitionPtr> trajectory;
    ksFaceDefinitionPtr roundingFace;
    bool needRework;
};

double getCylinderOrTorusRadius(ksFaceDefinitionPtr face);
bool faceNeedRework(ksFaceDefinitionPtr roundingFace);
bool targetNeedRework(RoundingEdgeOnPrintFaceTarget target);

std::list<RoundingEdgeOnPrintFaceTarget> getRoundingEdgesOnPrintFaceTargets(ksPartPtr part, PrintSurface printSurface, ReworkType reworkType);
void drawSketch(Sketch sketch, RoundingEdgeOnPrintFaceTarget target, double overhangThreshold);
void optimizeRoundingEdgesOnPrintFace(KompasObjectPtr kompas, ksPartPtr part, PrintSurface printSurface, const Settings& settings,
        ReworkType reworkType);

#endif /* ROUNDING_EDGES_ON_PRINT_FACE_HPP */

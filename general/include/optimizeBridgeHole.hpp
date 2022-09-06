#ifndef OPTIMIZE_BRIDGE_HOLE_HPP
#define OPTIMIZE_BRIDGE_HOLE_HPP

#include "stdafx.h"

#include <list>

struct BridgeHoleFillTarget {
    ksLoopPtr loop;
    ksFaceDefinitionPtr face;
};

struct BridgeHoleBuildTarget {
    ksLoopPtr innerLoop;
    ksLoopPtr outerLoop;
    ksFaceDefinitionPtr face;
};

enum class HoleType {
    CIRCLE,
    NOT_CIRCLE,
    ALL,
};

bool loopIsCircle(ksLoopPtr loop);
bool checkFaceWithHole(ksFaceDefinitionPtr face, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer);
bool isHoleDirect(ksFaceDefinitionPtr face, ksLoopPtr loop, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer);
bool checkHoleLoop(ksDocument3DPtr document3d, ksFaceDefinitionPtr face, ksLoopPtr loop, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer);

ksEntityPtr cutExtrusion(ksPartPtr part, ksEntityPtr sketchEntity, bool normalDirection, double depth);

std::list<BridgeHoleFillTarget> getBridgeHoleFillTargets(ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace, HoleType holeType);
void fillBridgeHoles(ksPartPtr part, std::list<BridgeHoleFillTarget> bridgeHoleTargets, double extrusionDepth);
void optimizeBridgeHoleFill(ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace, double extrusionDepth, HoleType holeType);

bool isOuterLoopForBuild(ksLoopPtr loop);
void drawLoopProjection(ksSketchDefinitionPtr sketchDef, ksLoopPtr loop);
std::list<BridgeHoleBuildTarget> getBridgeHoleBuildTargets(ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace);
ICirclePtr drawThinCircleProjection(Sketch sketch, ksPartPtr part, BridgeHoleBuildTarget target);
void bridgeHoleBuildCircleDrawSketch1(Sketch sketch, ICirclePtr innerCircle, BridgeHoleBuildTarget target);
void closeContour(ILineSegmentsPtr lineSegments, std::list<double> points, double y);
void bridgeHoleBuildNotCircleDrawSketch1(KompasObjectPtr kompas, Sketch sketch, ICirclePtr innerCircle, BridgeHoleBuildTarget target);
void bridgeHoleBuildDrawSketch2(KompasObjectPtr kompas, Sketch sketch, double centerX, double centerY, double radius, int angleCount);
void buildBridgeHoles(KompasObjectPtr kompas, ksPartPtr part, std::list<BridgeHoleBuildTarget> bridgeHoleTargets, double stepDepth);
void optimizeBridgeHoleBuild(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace, double stepDepth);

#endif /* OPTIMIZE_BRIDGE_HOLE_HPP */

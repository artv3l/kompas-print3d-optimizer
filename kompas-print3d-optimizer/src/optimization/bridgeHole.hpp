#ifndef BRIDGE_HOLE_HPP
#define BRIDGE_HOLE_HPP

#include <list>
#include <utility>

#include "kapiwrap/Sketch.hpp"

class Settings;

struct BridgeHoleFillTarget {
    kapi::ksLoopPtr loop;
    kapi::ksFaceDefinitionPtr face;
};

struct BridgeHoleBuildTarget {
    kapi::ksLoopPtr innerLoop;
    kapi::ksLoopPtr outerLoop;
    kapi::ksFaceDefinitionPtr face;
};

enum class HoleType {
    CIRCLE,
    NOT_CIRCLE,
    ALL,
};

struct MergePointInfo {
    double x, y;
    kapi::IDrawingObjectPtr drawingObject;
    int drawingObjectIndex;
};

struct Sketch1NotCircleInfo {
    Sketch sketch;
    kapi::ILinePtr line1, line2;
    std::list<MergePointInfo>& points1, & points2;
};

bool loopIsCircle(kapi::ksLoopPtr loop);
bool checkFaceWithHole(kapi::ksFaceDefinitionPtr face, kapi::ksFaceDefinitionPtr printFace, kapi::ksMeasurerPtr measurer);
bool isHoleDirect(kapi::ksFaceDefinitionPtr face, kapi::ksLoopPtr loop, kapi::ksFaceDefinitionPtr printFace, kapi::ksMeasurerPtr measurer);
bool checkHoleLoop(kapi::ksDocument3DPtr document3d, kapi::ksFaceDefinitionPtr face, kapi::ksLoopPtr loop, kapi::ksFaceDefinitionPtr printFace, kapi::ksMeasurerPtr measurer);

kapi::ksEntityPtr cutExtrusion(kapi::ksPartPtr part, kapi::ksEntityPtr sketchEntity, bool normalDirection, Settings& settings, int multiplier);

std::list<BridgeHoleFillTarget> getBridgeHoleFillTargets(kapi::ksDocument3DPtr document3d, kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace, HoleType holeType);
kapi::ksEntityPtr fillBridgeHoles(kapi::KompasObjectPtr kompas, kapi::ksPartPtr part, std::list<BridgeHoleFillTarget> bridgeHoleTargets, Settings& settings);
kapi::ksEntityPtr optimizeBridgeHoleFill(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, kapi::ksPartPtr part, Settings& settings, HoleType holeType);

bool isOuterLoopForBuild(kapi::ksLoopPtr loop);
std::list<BridgeHoleBuildTarget> getBridgeHoleBuildTargets(kapi::ksDocument3DPtr document3d, kapi::ksPartPtr part, kapi::ksFaceDefinitionPtr printFace);
void drawLoopProjection(kapi::ksSketchDefinitionPtr sketchDef, kapi::ksLoopPtr loop);
kapi::ICirclePtr drawThinInnerCircleProjection(Sketch sketch, BridgeHoleBuildTarget target);
void bridgeHoleBuildCircleDrawSketch1(Sketch sketch, kapi::ICirclePtr innerCircle, BridgeHoleBuildTarget target);
void closeContour(kapi::ILineSegmentsPtr lineSegments, std::list<MergePointInfo> points);
std::pair<kapi::ILinePtr, kapi::ILinePtr> drawBasicLines(Sketch sketch, kapi::ICirclePtr innerCircle, double angle);
bool pointInsideInterval(kapi::ksMathematic2DPtr math2d, double x, double y, kapi::ILinePtr line1, kapi::ILinePtr line2);
void processLineSegment(Sketch1NotCircleInfo info, kapi::ILineSegmentPtr lineSegment);
void processArc(Sketch1NotCircleInfo info, kapi::IArcPtr arc);
void bridgeHoleBuildNotCircleDrawSketch1(kapi::KompasObjectPtr kompas, Sketch sketch, kapi::ICirclePtr innerCircle, BridgeHoleBuildTarget target);
void bridgeHoleBuildDrawSketch2(kapi::KompasObjectPtr kompas, Sketch sketch, BridgeHoleBuildTarget target, int angleCount);
kapi::ksEntityPtr buildBridgeHoles(kapi::KompasObjectPtr kompas, kapi::ksPartPtr part, std::list<BridgeHoleBuildTarget> bridgeHoleTargets, Settings& settings);
kapi::ksEntityPtr optimizeBridgeHoleBuild(kapi::KompasObjectPtr kompas, kapi::ksDocument3DPtr document3d, kapi::ksPartPtr part, Settings& settings);

#endif /* BRIDGE_HOLE_HPP */

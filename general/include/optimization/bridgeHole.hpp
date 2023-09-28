#ifndef BRIDGE_HOLE_HPP
#define BRIDGE_HOLE_HPP

#include <list>
#include <utility>

#include "apiutil/Sketch.hpp"

class Settings;

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

struct MergePointInfo {
    double x, y;
    IDrawingObjectPtr drawingObject;
    int drawingObjectIndex;
};

struct Sketch1NotCircleInfo {
    Sketch sketch;
    ILinePtr line1, line2;
    std::list<MergePointInfo>& points1, & points2;
};

bool loopIsCircle(ksLoopPtr loop);
bool checkFaceWithHole(ksFaceDefinitionPtr face, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer);
bool isHoleDirect(ksFaceDefinitionPtr face, ksLoopPtr loop, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer);
bool checkHoleLoop(ksDocument3DPtr document3d, ksFaceDefinitionPtr face, ksLoopPtr loop, ksFaceDefinitionPtr printFace, ksMeasurerPtr measurer);

ksEntityPtr cutExtrusion(ksPartPtr part, ksEntityPtr sketchEntity, bool normalDirection, Settings& settings, int multiplier);

std::list<BridgeHoleFillTarget> getBridgeHoleFillTargets(ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace, HoleType holeType);
ksEntityPtr fillBridgeHoles(KompasObjectPtr kompas, ksPartPtr part, std::list<BridgeHoleFillTarget> bridgeHoleTargets, Settings& settings);
ksEntityPtr optimizeBridgeHoleFill(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, Settings& settings, HoleType holeType);

bool isOuterLoopForBuild(ksLoopPtr loop);
std::list<BridgeHoleBuildTarget> getBridgeHoleBuildTargets(ksDocument3DPtr document3d, ksPartPtr part, ksFaceDefinitionPtr printFace);
void drawLoopProjection(ksSketchDefinitionPtr sketchDef, ksLoopPtr loop);
ICirclePtr drawThinInnerCircleProjection(Sketch sketch, BridgeHoleBuildTarget target);
void bridgeHoleBuildCircleDrawSketch1(Sketch sketch, ICirclePtr innerCircle, BridgeHoleBuildTarget target);
void closeContour(ILineSegmentsPtr lineSegments, std::list<MergePointInfo> points);
std::pair<ILinePtr, ILinePtr> drawBasicLines(Sketch sketch, ICirclePtr innerCircle, double angle);
bool pointInsideInterval(ksMathematic2DPtr math2d, double x, double y, ILinePtr line1, ILinePtr line2);
void processLineSegment(Sketch1NotCircleInfo info, ILineSegmentPtr lineSegment);
void processArc(Sketch1NotCircleInfo info, IArcPtr arc);
void bridgeHoleBuildNotCircleDrawSketch1(KompasObjectPtr kompas, Sketch sketch, ICirclePtr innerCircle, BridgeHoleBuildTarget target);
void bridgeHoleBuildDrawSketch2(KompasObjectPtr kompas, Sketch sketch, BridgeHoleBuildTarget target, int angleCount);
ksEntityPtr buildBridgeHoles(KompasObjectPtr kompas, ksPartPtr part, std::list<BridgeHoleBuildTarget> bridgeHoleTargets, Settings& settings);
ksEntityPtr optimizeBridgeHoleBuild(KompasObjectPtr kompas, ksDocument3DPtr document3d, ksPartPtr part, Settings& settings);

#endif /* BRIDGE_HOLE_HPP */
